#include "app_config.h"

#include "gd32f30x.h"

#include "RUF_X_demo.h"
#include "show_UI.h"
#include "algorithmTask.h"
#include "watchdogTask.h"

#include "freertos_resources.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "ui.h"

#include <math.h>

#if USE_UART_LOG
#include <stdio.h>
#endif

// ---------------------------  接收数据刷新  ----------------------------

extern algo_state_t g_algo;

algo_out_t out = {0.0f, 0.0f, 0.0f, 0.0f};

void app_ui_update_flow(const algo_out_t *d)
{
    // 0~100
    float sq = d->sq;
    float v_mps = d->v_mps;
    float q_Lpmin = d->q_m3s * 1000 * 60;
    double q_total_L = d->q_total_m3 * 1000;

    int val = (int)(v_mps / 5.0f * 100);

    int int_part = (int)sq;
    int frac_part = (int)(fabsf(sq - int_part) * 1000.0f);

    lv_arc_set_value(ui_Arc1, val);
    lv_label_set_text_fmt(ui_SQIndicator, "SQ: %d", int_part);

    int_part = (int)q_Lpmin;
    frac_part = (int)(fabsf(q_Lpmin - int_part) * 1000.0f);

    lv_label_set_text_fmt(ui_instFlowIndicator, "inst: %d.%03d L/min", int_part, frac_part);

    int_part = (int)q_total_L;
    frac_part = (int)(fabsf((float)q_total_L - int_part) * 1000.0f);

    lv_label_set_text_fmt(ui_totalFlowIndicator, "total: %d.%03d L", int_part, frac_part);

    int_part = (int)v_mps;
    frac_part = (int)(fabsf(v_mps - int_part) * 1000.0f);

    lv_label_set_text_fmt(ui_FlowVector, "%d.%03d m/s", int_part, frac_part);
}

void gui_poll_message(void)
{
    while (xQueueReceive(xQueue_AlgoOut, &out, 0) == pdTRUE)
    {
        app_ui_update_flow(&out);
    }
}

// -------------------------- 按键-场景控制刷新 ------------------------
typedef enum
{
    UI_SCENE_MAIN = 0,
    UI_SCENE_CONFIG,
    UI_SCENE_SET,
    UI_SCENE_STATE
} ui_scene_t;

static ui_scene_t g_ui_scene = UI_SCENE_MAIN;

typedef enum
{
    CONFIG_IDLE = 0,
    CONFIG_MATERIAL_OPEN,
    CONFIG_DIAMETER_OPEN

} config_state_t;

static config_state_t g_config_state = CONFIG_IDLE;

void ui_handle_main(key_event_t evt)
{
    switch (evt)
    {
    /* K1 → Screen2 */
    case KEY_EVENT_K1:

        lv_scr_load(ui_Screen2);
        g_ui_scene = UI_SCENE_CONFIG;
        g_config_state = CONFIG_IDLE;

        break;

    case KEY_EVENT_K2:
        break;

    case KEY_EVENT_K3:
        break;

    case KEY_EVENT_K4:
        break;

    default:
        break;
    }
}

void ui_handle_config(key_event_t evt)
{
    switch (g_config_state)
    {

        /* ================= CONFIG 主界面 ================= */

    case CONFIG_IDLE:

        switch (evt)
        {
        /* K1 → Screen3 */
        case KEY_EVENT_K1:

            lv_scr_load(ui_Screen3);
            g_ui_scene = UI_SCENE_SET;

            break;

        /* 打开 material dropdown */
        case KEY_EVENT_K2:

            lv_dropdown_open(ui_materialDropDown);
            g_config_state = CONFIG_MATERIAL_OPEN;

            break;

        /* 打开 diameter dropdown */
        case KEY_EVENT_K3:

            lv_dropdown_open(ui_diameterDropDown);
            g_config_state = CONFIG_DIAMETER_OPEN;

            break;

        /* 返回 MAIN */
        case KEY_EVENT_K4:

            lv_scr_load(ui_Screen1);
            g_ui_scene = UI_SCENE_MAIN;

            break;

        default:
            break;
        }

        break;

        /* ================= MATERIAL dropdown ================= */

    case CONFIG_MATERIAL_OPEN:

        switch (evt)
        {
        /* 选择 */
        case KEY_EVENT_K1:
        {
            uint16_t id = lv_dropdown_get_selected(ui_materialDropDown);

            switch (id)
            {
            case 0:
                material_set(METAL);
                break;

            case 1:
                material_set(PLASTIC);
                break;

            case 2:
                material_set(ALUMINUM);
                break;
            }

            lv_dropdown_close(ui_materialDropDown);
            g_config_state = CONFIG_IDLE;

            clear_data();
        }
        break;

        /* 向上 */
        case KEY_EVENT_K2:
        {
            uint16_t sel = lv_dropdown_get_selected(ui_materialDropDown);
            uint16_t cnt = lv_dropdown_get_option_cnt(ui_materialDropDown);

            if (sel == 0)
                sel = cnt - 1;
            else
                sel--;

            lv_dropdown_set_selected(ui_materialDropDown, sel);
        }
        break;

        /* 向下 */
        case KEY_EVENT_K3:
        {
            uint16_t sel = lv_dropdown_get_selected(ui_materialDropDown);
            uint16_t cnt = lv_dropdown_get_option_cnt(ui_materialDropDown);

            sel++;
            if (sel >= cnt)
                sel = 0;

            lv_dropdown_set_selected(ui_materialDropDown, sel);
        }
        break;

        /* 返回 CONFIG */
        case KEY_EVENT_K4:

            lv_dropdown_close(ui_materialDropDown);
            g_config_state = CONFIG_IDLE;

            break;

        default:
            break;
        }

        break;

        /* ================= DIAMETER dropdown ================= */

    case CONFIG_DIAMETER_OPEN:

        switch (evt)
        {
        /* 选择 */
        case KEY_EVENT_K1:
        {
            uint16_t id = lv_dropdown_get_selected(ui_diameterDropDown);

            switch (id)
            {
            case 0:
                diameter_set_dn(35.0f);
                break;

            case 1:
                diameter_set_dn(25.0f);
                break;

            case 2:
                diameter_set_dn(20.0f);
                break;

            case 3:
                diameter_set_dn(15.0f);
                break;

            case 4:
                diameter_set_dn(10.0f);
                break;
            }

            lv_dropdown_close(ui_diameterDropDown);
            g_config_state = CONFIG_IDLE;

            clear_data();
        }
        break;

        /* 向上 */
        case KEY_EVENT_K2:
        {
            uint16_t sel = lv_dropdown_get_selected(ui_diameterDropDown);
            uint16_t cnt = lv_dropdown_get_option_cnt(ui_diameterDropDown);

            if (sel == 0)
                sel = cnt - 1;
            else
                sel--;

            lv_dropdown_set_selected(ui_diameterDropDown, sel);
        }
        break;

        /* 向下 */
        case KEY_EVENT_K3:
        {
            uint16_t sel = lv_dropdown_get_selected(ui_diameterDropDown);
            uint16_t cnt = lv_dropdown_get_option_cnt(ui_diameterDropDown);

            sel++;
            if (sel >= cnt)
                sel = 0;

            lv_dropdown_set_selected(ui_diameterDropDown, sel);
        }
        break;

        /* 返回 CONFIG */
        case KEY_EVENT_K4:

            lv_dropdown_close(ui_diameterDropDown);
            g_config_state = CONFIG_IDLE;

            break;

        default:
            break;
        }

        break;
    }
}

void ui_handle_set(key_event_t evt)
{
    switch (evt)
    {
    /* K1 → Screen4 */
    case KEY_EVENT_K1:

        lv_scr_load(ui_Screen4);
        g_ui_scene = UI_SCENE_STATE;

        lv_label_set_text_fmt(ui_TextAlgoStateShow, "RUNNING");

        switch (g_algo.material)
        {
        case METAL:
            lv_label_set_text_fmt(ui_TextMaterialShow, "METAL");
            break;

        case PLASTIC:
            lv_label_set_text_fmt(ui_TextMaterialShow, "PVC");
            break;

        case ALUMINUM:
            lv_label_set_text_fmt(ui_TextMaterialShow, "ALUMINUM");
            break;

        default:
            break;
        }

        lv_label_set_text_fmt(ui_TextDiameterShow, "DN %d", (int)g_algo.pipe_dn);

        break;

    /* Reset 按钮 */
    case KEY_EVENT_K2:

        lv_event_send(ui_ButtonReset,
                      LV_EVENT_CLICKED,
                      NULL);

        clear_data();

        break;

    /* Zero Drift Learn */
    case KEY_EVENT_K3:

        lv_event_send(ui_ButtonZeroDriftLearn,
                      LV_EVENT_CLICKED,
                      NULL);

        zero_drift_learn_start();
        clear_data();

        break;

    /* 返回 CONFIG */
    case KEY_EVENT_K4:

        lv_scr_load(ui_Screen2);

        g_ui_scene = UI_SCENE_CONFIG;
        g_config_state = CONFIG_IDLE;

        break;

    default:
        break;
    }
}

void ui_handle_state(key_event_t evt)
{
    switch (evt)
    {
    case KEY_EVENT_K1:
        break;

    case KEY_EVENT_K2:
        break;

    case KEY_EVENT_K3:
        break;

    /* 返回 SET */
    case KEY_EVENT_K4:

        lv_scr_load(ui_Screen3);
        g_ui_scene = UI_SCENE_SET;

        break;

    default:
        break;
    }
}

void ui_handle_key(key_event_t evt)
{
    switch (g_ui_scene)
    {
    case UI_SCENE_MAIN:
        ui_handle_main(evt);
        break;

    case UI_SCENE_CONFIG:
        ui_handle_config(evt);
        break;

    case UI_SCENE_SET:
        ui_handle_set(evt);
        break;

    case UI_SCENE_STATE:
        ui_handle_state(evt);
        break;

    default:
        break;
    }
}

void gui_poll_key(void)
{
    key_event_t evt;

    while (xQueueReceive(xQueue_KeyEvent, &evt, 0) == pdTRUE)
    {
        ui_handle_key(evt);
    }
}
void vShow_UI_task(void *pvParameter)
{
    lv_init();           /* lvgl系统初始化 */
    lv_port_disp_init(); /* lvgl显示接口初始化 */

    ui_init();
    (void)pvParameter;
    uint32_t cnt = 0;

#if USE_UART_LOG
    TickType_t last_log = xTaskGetTickCount();
#endif

    while (1)
    {
        lv_timer_handler();

        gui_poll_key();

        if (++cnt % 10 == 0)
        {
            gui_poll_message();
        }

#if USE_UART_LOG
        {
            TickType_t now = xTaskGetTickCount();
            if ((now - last_log) >= pdMS_TO_TICKS(2000))
            {
                last_log = now;
                vPrintTaskStackInfo();
            }
        }
#endif
        watchdog_kick_ui();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
