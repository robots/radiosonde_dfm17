#include "platform.h"

#include "tinyprintf.h"

#include "tehu.h"
#include "LiquidCrystal_I2C.h"
#include "ui.h"

static char line1[17];
static char line2[17];

void ui_init(void)
{
	LCDI2C_init(0x27, 16, 2);

	LCDI2C_backlight();
	LCDI2C_clear();
}

void ui_splash(void)
{
	LCDI2C_write_String("radiosonde");

	//delay_ms(600);
	systime_delay(600);
}

void ui_update(void)
{
	uint32_t te = tehu_get_t_e() * 1000;
//	uint32_t to = tehu_get_t_o() * 100;
	uint32_t h = tehu_get_hum() * 100;
	uint32_t rh = tehu_get_relhum() * 100;

	tfp_sprintf(line1, "T=%3d.%03d        ", te / 1000, te % 1000);
	tfp_sprintf(line2, "H=%2d.%02d RH=%2d.%02d", h / 100, h % 100, rh / 100, rh % 100);
	LCDI2C_setCursor(0,0);
	LCDI2C_write_String(line1);
	LCDI2C_setCursor(0,1);
	LCDI2C_write_String(line2);
}
