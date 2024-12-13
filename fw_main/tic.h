#ifndef TIC_h_
#define TIC_h_

enum {
	TIC_CH_TEMP_E,
	TIC_CH_TEMP_O,
	TIC_CH_REF_6,
	TIC_CH_REF_A,
	TIC_CH_HUM_D,
};

void tic_init(void);
void tic_select_channel(int c);
void tic_start(void);
void tic_stop(void);
float tic_get_result(void);

#endif
