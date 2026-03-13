#include "dac.h"

void dac_config(void)
{
    /* initialize DAC */
    dac_deinit(DAC0);
    /* DAC trigger disable */
    dac_trigger_disable(DAC0, DAC_OUT0);
    dac_trigger_disable(DAC0, DAC_OUT1);
    /* DAC wave mode config */
    dac_wave_mode_config(DAC0, DAC_OUT0, DAC_WAVE_DISABLE);
    dac_wave_mode_config(DAC0, DAC_OUT1, DAC_WAVE_DISABLE);

    /* DAC enable */
    dac_concurrent_enable(DAC0);
}

static double vout_to_vdac(double vout)
{
    double  vdac;
    double  r_high = 499000;
    double  r_low = 20000;   
    double  r_dac = 100000;
    double  vref = 0.6;

    double A = vref * (1 + r_high/r_low + r_high/r_dac);
    double B = r_high/r_dac;
    vdac = (A - vout) / B;


    return vdac;
}
void dac_set_fir_vol(double vol)
{
    double vdac = vout_to_vdac(vol);
    dac_data_set(DAC0, DAC_OUT1, DAC_ALIGN_12B_R, (uint16_t)(vdac*4095.0f/3.3));
}

void dac_set_com_vol(double vol)
{
    // double vdac = vout_to_vdac(vol);
    // dac_data_set(DAC0, DAC_OUT0, DAC_ALIGN_12B_R, (uint16_t)(vdac*4095.0f)/3.3);
}