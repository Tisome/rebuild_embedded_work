#ifndef __BSP_OS_SUPPORT_H__
#define __BSP_OS_SUPPORT_H__

#define OS_SUPPORTING

/* From OS   Layer：  OS_Delay        */
#ifdef OS_SUPPORTING
typedef struct
{
    void (*pf_rtos_yield)(const uint32_t);          /*OS Not-Blocking Delay  */
}yield_interface_t;

#endif //End of OS_SUPPORTING


/* From Core Layer：  TimeBase        */
typedef struct
{
    uint32_t (*pf_get_tick_count)(void);            /*Get Tick number interf.*/
} timebase_interface_t;

#endif