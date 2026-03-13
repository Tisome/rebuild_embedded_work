#include"iwdg.h"



void my_iwdg_init(void)
{
    // fwdgt_config(4096*8, FWDGT_PSC_DIV4);

    /* After 1.6 seconds to generate a reset */
    fwdgt_enable();
}