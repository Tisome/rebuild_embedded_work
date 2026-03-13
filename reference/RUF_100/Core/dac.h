#ifndef __DAC_H__
#define __DAC_H__
#include "gd32f30x.h"

void dac_config(void);

void dac_set_fir_vol(double vol);

void dac_set_com_vol(double vol);
#endif
