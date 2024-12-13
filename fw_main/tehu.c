
#include "platform.h"

#include "systime.h"
#include "led.h"

#include "eeprom.h"
#include "tic.h"
#include "tehu.h"

#include <math.h>

#define TEHU_SAMPLE_TIME 150

static float coef_D[5] = {
	-552.8492f,
	11680.487f,
	-15933.65f,
	-280383.5f,
	1.0f,
};

static float coef_E[6] = {
	-74.01539f,
	-13.508989f,
	0.6774659f,
	-0.044331126f,
	3.8692253E-4f,
	-3.4753332E-4f,
};

static float coef_O[6] = {
	-67.31276f,
	-14.81795f,
	1.3730069f,
	0.2794185f,
	0.07187398f,
	0.0049287053f,
};

static float coef_7 = 0.3312359f;
static float coef_6 = 0.020488238f;

static const int seq2ch[5] = {
	TIC_CH_REF_6,
	TIC_CH_REF_A,
	TIC_CH_TEMP_O,
	TIC_CH_TEMP_E,
	TIC_CH_HUM_D,
};

static float meas[5] = {
	0.031,
	0.025,
	0.020,
	0.330,
	0.063,
	};

static size_t tehu_seq;
static size_t tehu_state;
static uint32_t tehu_state_time;

static float result_t_e;
static float result_t_o;
static float result_h_d;
static float result_rh;
static int results_new;

static int load_coefs(void);
static float apply_coef(float raw, float *coef, size_t count);
static float calc_rh(void);
static float convert_humi(void);
static float convert_temp(int is_e);

void tehu_init(void)
{
	tehu_seq = 0;
	tehu_state = 0;
	tehu_state_time = systime_get();

	if (load_coefs()) {
		led_set(0, LED_3BLINK);	
	}
}

int tehu_is_new_results(void)
{
	int ret = results_new;
	results_new = 0;
	return ret;
}

static int load_coefs(void)
{
	int ret;
	uint8_t ids[16];
	uint8_t lens[8];

	// read sequence from EE
	ret = eeprom_read_buf(0xa0, ids, 16);
	if (ret != 0) return 1;

	// read lengths
	ret = eeprom_read_buf(0xb0, lens, 8);
	if (ret != 0) return 1;

	uint16_t addr = 0xb8;
	for(size_t i = 0; i < 16; i++) {
		size_t l;
		// decode length
		if (i % 2 == 0) {
			l = lens[i/2] >> 4;
		} else {
			l = lens[i/2] & 0xf;
		}

		// select table according to ID
		float *tab = NULL;
		size_t tablen = 0;
		if (ids[i] == 'E') {
			tab = coef_E;
			tablen = ARRAY_SIZE(coef_E);
		} else if (ids[i] == 'O') {
			tab = coef_O;
			tablen = ARRAY_SIZE(coef_O);
		} else if (ids[i] == 'D') {
			tab = coef_D;
			tablen = ARRAY_SIZE(coef_D);
		} else if (ids[i] == 0x06) {
			tab = &coef_6;
			tablen = 1;
		} else if (ids[i] == 0x07) {
			tab = &coef_7;
			tablen = 1;
		}

		// extract coeficients
		for (size_t j = 0; j < l; j++) {
			double coef;
			ret = eeprom_read_buf(addr, (void *)&coef, 8);
			if (ret != 0) return 1;

			// there are more coefs for 'O' (element is zero, so ignore)
			if (tablen > 0) {
				tab[0] = (float)coef;
				tablen --;
				tab++;
			}

			// advance to next coef
			addr += 8;
		}
	}
	return 0;
}

void tehu_periodic(void)
{
	if (tehu_state == 0)
	{
		tic_select_channel(seq2ch[tehu_seq]);

		tic_start();

		tehu_state = 1;
		tehu_state_time = systime_get();
	}
	else if (tehu_state == 1)
	{
		if (systime_get() - tehu_state_time > TEHU_SAMPLE_TIME)
		{
			tic_stop();

			meas[seq2ch[tehu_seq]] = tic_get_result();	
			tehu_state = 2;
			tehu_state_time = systime_get();
		}
	}
	else if (tehu_state == 2)
	{
		if (tehu_seq == 2)
		{
			result_t_e = convert_temp(1);
		}
		else if (tehu_seq == 3)
		{
			result_t_o = convert_temp(0);
		}
		else if (tehu_seq == 4)
		{
			result_h_d = convert_humi();
			result_rh = calc_rh();
		}

		tehu_state = 0;
		tehu_seq += 1;
		if (tehu_seq >= ARRAY_SIZE(meas)) {
			tehu_seq = 0;
			results_new = 1;
		}
	}
}

float tehu_get_t_e(void)
{
	return result_t_e;
}

float tehu_get_t_o(void)
{
	return result_t_o;
}

float tehu_get_hum(void)
{
	return result_h_d;
}

float tehu_get_relhum(void)
{
	return result_rh;
}

static float apply_coef(float raw, float *coef, size_t count)
{
	float x = raw * coef[count-1] + coef[count-2];

	for(int i = count-3; i >= 0; i--)
	{
		x = x * raw;
		x += coef[i];
	}

	return x;
}

static float calc_rh(void)
{
	float t1 = (result_t_e * 17.5043f) / (result_t_e + 241.2f);
	float t2 = (result_t_o * 17.5043f) / (result_t_o + 241.2f);

	float x = expf(t1-t2);

	x = result_h_d * 100.0f / ((x - 1.0f) * 100.0f + 100.0f);

	return x;
}

static float convert_humi(void)
{
	float x = meas[TIC_CH_HUM_D];

	x = apply_coef(x, coef_D, ARRAY_SIZE(coef_D));

	float dt = result_t_o - 25.0f;
	x -= dt * -0.0147630535f;
	x /= dt * -0.0030510311f + 1.0f;

	if (result_t_o < 0.0f)
	{
			x /= result_t_o * result_t_o * -5.9642734E-5f + 1.0f;
	}

	return x;
}

static float convert_temp(int is_e)
{
	float m;

	if (is_e) {
		m = (float)meas[TIC_CH_TEMP_E];
	} else {
		m = (float)meas[TIC_CH_TEMP_O];
	}

	float x = (1.0f / m - 1.0f / meas[TIC_CH_REF_A]) * (1.0f / coef_6 - 1.0f / coef_7) / (1.0f / (float)meas[TIC_CH_REF_6] - 1.0f / (float)meas[TIC_CH_REF_A]);

	x = 1.0f / (x + (1.0f / coef_7)) - coef_6;

	x = logf(x);

	if (is_e) {
		return apply_coef(x, coef_E, ARRAY_SIZE(coef_E));
	}	else {
		return apply_coef(x, coef_O, ARRAY_SIZE(coef_O));
	}
}
