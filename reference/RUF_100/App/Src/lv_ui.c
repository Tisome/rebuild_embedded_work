#include"lv_ui.h"
#include "data.h"
#include "FreeRTOS.h"
#include "semphr.h"

// 单位转换系数定义
#define M3_TO_L  (1000.0)
#define M3_TO_ML (1000000.0)
#define M3H_TO_LMIN (1000.0 / 60.0)
#define M3H_TO_MLMIN (1000000.0 / 60.0)
#define M3H_TO_LH (1000.0)

lv_ui_total* g_lv_ui_total;
lv_ui_menu* g_lv_ui_menu;
lv_ui_display* g_lv_ui_display;
lv_ui_edit* g_lv_ui_edit;

extern const lv_font_t lv_font_interttf_16;
extern const lv_font_t lv_font_interttf_24;
extern const lv_font_t lv_font_alimama_16;
extern const lv_font_t my_font_alimama_20;

extern const lv_font_t lv_font_montserrat_38;
extern const lv_font_t lv_font_montserrat_24;
extern const lv_font_t lv_font_montserrat_20;

void ui_menu_total_init(void)
{
    g_lv_ui_total = malloc(sizeof(lv_ui_total));
    memset(g_lv_ui_total, 0, sizeof(lv_ui_total));
    g_lv_ui_total->screen = lv_scr_act();
    lv_obj_set_style_border_width(g_lv_ui_total->screen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_lv_ui_total->screen, 255, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_lv_ui_total->screen, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(g_lv_ui_total->screen, LV_SCROLLBAR_MODE_OFF);

    //主菜单和标题初始化
    //Write codes List_1
    g_lv_ui_total->list = lv_obj_create(g_lv_ui_total->screen);
    lv_obj_set_pos(g_lv_ui_total->list, 0, 0);
    lv_obj_set_size(g_lv_ui_total->list, 240, 240);
    lv_obj_set_scrollbar_mode(g_lv_ui_total->list, LV_SCROLLBAR_MODE_OFF);

    //Write style for List_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(g_lv_ui_total->list, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_lv_ui_total->list, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(g_lv_ui_total->list, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_lv_ui_total->list, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
}
void ui_menu_list_init(void)
{

    g_lv_ui_menu = malloc(sizeof(lv_ui_menu));
    memset(g_lv_ui_menu, 0, sizeof(lv_ui_menu));
    //Write codes List_1_btn_1
    g_lv_ui_menu->menu_list_title = lv_label_create(g_lv_ui_total->list);
    lv_label_set_text(g_lv_ui_menu->menu_list_title,"MAIN MENU");
    lv_label_set_long_mode(g_lv_ui_menu->menu_list_title, LV_LABEL_LONG_WRAP);
    lv_obj_align(g_lv_ui_menu->menu_list_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_pad_all(g_lv_ui_menu->menu_list_title, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(g_lv_ui_menu->menu_list_title, LV_PCT(100));
    lv_obj_set_pos(g_lv_ui_menu->menu_list_title, 0, 0);
    lv_obj_set_size(g_lv_ui_menu->menu_list_title, 100, 60);

    //Write style for List_1_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(g_lv_ui_menu->menu_list_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_lv_ui_menu->menu_list_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(g_lv_ui_menu->menu_list_title, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(g_lv_ui_menu->menu_list_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_lv_ui_menu->menu_list_title, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);

    //chinese
    lv_obj_set_style_text_font(g_lv_ui_menu->menu_list_title, &my_font_alimama_20, LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_set_style_text_opa(g_lv_ui_menu->menu_list_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(g_lv_ui_menu->menu_list_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    //Write codes List_1_btn_1
    g_lv_ui_menu->menu_list_page = lv_label_create(g_lv_ui_total->list);
    lv_label_set_text(g_lv_ui_menu->menu_list_page,"<1/1>");
    lv_label_set_long_mode(g_lv_ui_menu->menu_list_page, LV_LABEL_LONG_WRAP);
    lv_obj_align(g_lv_ui_menu->menu_list_page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_pad_all(g_lv_ui_menu->menu_list_page, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(g_lv_ui_menu->menu_list_page, LV_PCT(100));
    lv_obj_set_pos(g_lv_ui_menu->menu_list_page, 150, 0);
    lv_obj_set_size(g_lv_ui_menu->menu_list_page, 60, 60);

    //Write style for List_1_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(g_lv_ui_menu->menu_list_page, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_lv_ui_menu->menu_list_page, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(g_lv_ui_menu->menu_list_page, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(g_lv_ui_menu->menu_list_page, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_lv_ui_menu->menu_list_page, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);

    //chinese
    lv_obj_set_style_text_font(g_lv_ui_menu->menu_list_page, &my_font_alimama_20, LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_set_style_text_opa(g_lv_ui_menu->menu_list_page, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(g_lv_ui_menu->menu_list_page, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    //Write codes List_1_cont_1
    g_lv_ui_menu->menu_list_total = lv_obj_create(g_lv_ui_total->list);
    lv_obj_set_pos(g_lv_ui_menu->menu_list_total, 0, 25);
    lv_obj_set_size(g_lv_ui_menu->menu_list_total, 240, 215);
    lv_obj_set_scrollbar_mode(g_lv_ui_menu->menu_list_total, LV_SCROLLBAR_MODE_OFF);

    //Write style for List_1_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(g_lv_ui_menu->menu_list_total, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(g_lv_ui_menu->menu_list_total, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_lv_ui_menu->menu_list_total, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(g_lv_ui_menu->menu_list_total, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(g_lv_ui_menu->menu_list_total, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(g_lv_ui_menu->menu_list_total, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(g_lv_ui_menu->menu_list_total, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(g_lv_ui_menu->menu_list_total, 0, LV_PART_MAIN|LV_STATE_DEFAULT);


    //菜单项初始化
        //Write codes List_1_cont_2
    char* text[3] = {"Measurements show","Setting","Display frame rate"};
    for(int index=0;index<DEFAULT_ITEMS_PER_PAGE;index++)
    {
    g_lv_ui_menu->menu_list_cont[index] = lv_obj_create(g_lv_ui_menu->menu_list_total);
    lv_obj_set_pos(g_lv_ui_menu->menu_list_cont[index], 0, 70*index);
    lv_obj_set_size(g_lv_ui_menu->menu_list_cont[index], 228, 60);
    lv_obj_set_scrollbar_mode(g_lv_ui_menu->menu_list_cont[index], LV_SCROLLBAR_MODE_OFF);

    //Write style for List_1_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(g_lv_ui_menu->menu_list_cont[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(g_lv_ui_menu->menu_list_cont[index], 15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_lv_ui_menu->menu_list_cont[index], 20, LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(g_lv_ui_menu->menu_list_cont[index], lv_color_hex(0x2e2e2e), LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_set_style_bg_grad_dir(g_lv_ui_menu->menu_list_cont[index], LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(g_lv_ui_menu->menu_list_cont[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(g_lv_ui_menu->menu_list_cont[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(g_lv_ui_menu->menu_list_cont[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(g_lv_ui_menu->menu_list_cont[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(g_lv_ui_menu->menu_list_cont[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    //Write codes List_1_label_1
    g_lv_ui_menu->menu_label[index] = lv_label_create(g_lv_ui_menu->menu_list_cont[index]);
    lv_label_set_text(g_lv_ui_menu->menu_label[index], text[index]);
    lv_label_set_long_mode(g_lv_ui_menu->menu_label[index], LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(g_lv_ui_menu->menu_label[index], 0, 15);
    lv_obj_set_size(g_lv_ui_menu->menu_label[index], 180, 30);

    //Write style for List_1_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_lv_ui_menu->menu_label[index], lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(g_lv_ui_menu->menu_label[index], &lv_font_interttf_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(g_lv_ui_menu->menu_label[index], 240, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(g_lv_ui_menu->menu_label[index], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(g_lv_ui_menu->menu_label[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_align(g_lv_ui_menu->menu_label[index], LV_ALIGN_CENTER, 0, 0);
    }
}
void ui_menu_list_set(char** title,bool language)
{
    lv_label_set_text(g_lv_ui_menu->menu_list_title, title[language]);
}

void ui_menu_list_item_set(uint32_t index,char* text,bool is_select,bool languange)
{
    //Write codes List_1_cont_2 
    if(is_select)
    {
        lv_obj_set_style_border_color(g_lv_ui_menu->menu_list_cont[index], lv_color_hex(0xFF0000), LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(g_lv_ui_menu->menu_list_cont[index], 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    }
    else
    {
        lv_obj_set_style_border_color(g_lv_ui_menu->menu_list_cont[index], lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(g_lv_ui_menu->menu_list_cont[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    }
    lv_obj_set_style_bg_opa(g_lv_ui_menu->menu_list_cont[index], 20, LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(g_lv_ui_menu->menu_list_cont[index], lv_color_hex(0x2e2e2e), LV_PART_MAIN|LV_STATE_DEFAULT);


    lv_label_set_text(g_lv_ui_menu->menu_label[index], text);

    if(languange)
    {
        //chinese
        lv_obj_set_style_text_font(g_lv_ui_menu->menu_label[index], &my_font_alimama_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    }
    else
    {
        //english
        lv_obj_set_style_text_font(g_lv_ui_menu->menu_label[index], &lv_font_interttf_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    }
}

void ui_menu_list_item_page_set(char* index)
{
        lv_label_set_text(g_lv_ui_menu->menu_list_page, index);
}

void ui_menu_list_item_set_disappear(uint32_t index)
{

    lv_obj_set_style_border_color(g_lv_ui_menu->menu_list_cont[index], lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_lv_ui_menu->menu_list_cont[index], 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_set_style_bg_opa(g_lv_ui_menu->menu_list_cont[index], 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(g_lv_ui_menu->menu_list_cont[index], lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_label_set_text(g_lv_ui_menu->menu_label[index], "");
}


void ui_diplay_init(void)
{
    g_lv_ui_display = malloc(sizeof(lv_ui_display));
    memset(g_lv_ui_display, 0, sizeof(lv_ui_display));

    // 这个圆环可以根据流速大小动态改变 value
    g_lv_ui_display->arc_flow = lv_arc_create(g_lv_ui_total->list);
    lv_obj_set_size(g_lv_ui_display->arc_flow, 190, 190);
    lv_obj_set_pos(g_lv_ui_display->arc_flow, 0, 15);
    lv_obj_align(g_lv_ui_display->arc_flow, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_rotation(g_lv_ui_display->arc_flow, 135); // 起始角度
    lv_arc_set_bg_angles(g_lv_ui_display->arc_flow, 0, 270); // 缺口在下方
    lv_arc_set_value(g_lv_ui_display->arc_flow, 0); 
    lv_obj_remove_style(g_lv_ui_display->arc_flow, NULL, LV_PART_KNOB); // 移除旋钮，只要进度条
    lv_obj_clear_flag(g_lv_ui_display->arc_flow, LV_OBJ_FLAG_CLICKABLE); // 不可点击
    
    lv_obj_set_style_arc_color(g_lv_ui_display->arc_flow, lv_color_hex(0x333333), LV_PART_MAIN); // 轨道深灰
    lv_obj_set_style_arc_color(g_lv_ui_display->arc_flow, lv_color_hex(0xFF0000), LV_PART_INDICATOR); // 进度亮青
    lv_obj_set_style_arc_width(g_lv_ui_display->arc_flow, 10, LV_PART_MAIN);      // 细线条更显精致
    lv_obj_set_style_arc_width(g_lv_ui_display->arc_flow, 6, LV_PART_INDICATOR);

    g_lv_ui_display->label_sq = lv_label_create(g_lv_ui_total->list);

    lv_label_set_text(g_lv_ui_display->label_sq, "SQ: 0"); 
    lv_obj_align(g_lv_ui_display->label_sq, LV_ALIGN_TOP_LEFT, 5, 0); 
    lv_obj_set_size(g_lv_ui_display->label_sq,70, 15);
    lv_obj_set_pos(g_lv_ui_display->label_sq, 0, 0);
    lv_obj_set_style_text_color(g_lv_ui_display->label_sq, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);

    //标志位
    g_lv_ui_display->label_flag = lv_label_create(g_lv_ui_total->list);
    lv_label_set_text(g_lv_ui_display->label_flag, ""); 
    lv_obj_set_style_text_font(g_lv_ui_display->label_flag, &my_font_alimama_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_align(g_lv_ui_display->label_flag, LV_ALIGN_TOP_LEFT, 5, 0); 
    lv_obj_set_size(g_lv_ui_display->label_flag,70, 15);
    lv_obj_set_pos(g_lv_ui_display->label_flag, 150, 0);
    lv_obj_set_style_text_color(g_lv_ui_display->label_flag, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);

    g_lv_ui_display->label_instance_val = lv_label_create(g_lv_ui_display->arc_flow);
    lv_label_set_text(g_lv_ui_display->label_instance_val, "0.00");
    lv_obj_align(g_lv_ui_display->label_instance_val, LV_ALIGN_CENTER, 0, -10); 
    lv_obj_set_style_text_color(g_lv_ui_display->label_instance_val, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(g_lv_ui_display->label_instance_val, &lv_font_montserrat_38, LV_PART_MAIN|LV_STATE_DEFAULT);

    g_lv_ui_display->label_instance_unit = lv_label_create(g_lv_ui_display->arc_flow);
    lv_label_set_text(g_lv_ui_display->label_instance_unit, "m3/h");
    lv_obj_align(g_lv_ui_display->label_instance_unit, LV_ALIGN_CENTER, 0, 30); 
    lv_obj_set_style_text_color(g_lv_ui_display->label_instance_unit, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    
    g_lv_ui_display->label_total_val = lv_label_create(g_lv_ui_total->list);
    lv_label_set_text(g_lv_ui_display->label_total_val, "0.00");
    lv_obj_align(g_lv_ui_display->label_total_val, LV_ALIGN_BOTTOM_RIGHT, -60, 0); 
    lv_obj_set_style_text_color(g_lv_ui_display->label_total_val, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    
    g_lv_ui_display->label_total_unit = lv_label_create(g_lv_ui_total->list);
    lv_label_set_text(g_lv_ui_display->label_total_unit, "m3");
    lv_obj_align(g_lv_ui_display->label_total_unit, LV_ALIGN_BOTTOM_RIGHT, 0, 0); 
    lv_obj_set_style_text_color(g_lv_ui_display->label_total_unit, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
}

// 辅助数学函数：计算整数倍率 (10^n)
static int32_t get_scale_factor(uint8_t precision) {
    int32_t factor = 1;
    for(int i=0; i<precision; i++) factor *= 10;
    return factor;
}
void ui_edit_init()
{
    g_lv_ui_edit = malloc(sizeof(lv_ui_edit));
    memset(g_lv_ui_edit, 0, sizeof(lv_ui_edit));
    // --- 2. 标题 (参数名) ---
    g_lv_ui_edit->edit_title = lv_label_create(g_lv_ui_total->list);
    // lv_label_set_text(g_lv_ui_edit->edit_title, param->name);
    // 设置大字体 (如果没有 Font_20，请改回 lv_font_montserrat_14 或 default)
    lv_obj_set_style_text_font(g_lv_ui_edit->edit_title, &lv_font_montserrat_20, 0); 
    lv_obj_set_style_text_color(g_lv_ui_edit->edit_title, lv_color_white(), 0);
    lv_obj_set_pos(g_lv_ui_edit->edit_title, 0, 0);
    lv_obj_set_size(g_lv_ui_edit->edit_title, 200, 60);

    // --- 4. 核心组件 Spinbox ---
    g_lv_ui_edit->spinbox = lv_spinbox_create(g_lv_ui_total->list);
    // lv_spinbox_set_digit_format(g_lv_ui_edit->spinbox, param->digits, param->precision);
    lv_spinbox_set_range(g_lv_ui_edit->spinbox, 0, 999999); // 设置足够大的范围
    lv_spinbox_set_step(g_lv_ui_edit->spinbox, 1);                // 步进值
    lv_obj_set_width(g_lv_ui_edit->spinbox, 120);
    lv_obj_set_style_text_font(g_lv_ui_edit->spinbox, &lv_font_montserrat_24, 0); // 数字大字体
    lv_obj_set_style_text_align(g_lv_ui_edit->spinbox, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_set_style_border_width(g_lv_ui_edit->spinbox, 0, LV_PART_MAIN);   
    lv_obj_set_style_outline_width(g_lv_ui_edit->spinbox, 0, LV_PART_MAIN);  

    lv_obj_set_style_bg_opa(g_lv_ui_edit->spinbox, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_lv_ui_edit->spinbox, lv_color_hex(0x2e2e2e), LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(g_lv_ui_edit->spinbox, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    // 样式美化：选中位高亮颜色
    lv_obj_set_style_bg_color(g_lv_ui_edit->spinbox, lv_palette_main(LV_PALETTE_GREY), LV_PART_CURSOR); 
    lv_obj_set_pos(g_lv_ui_edit->spinbox, 0, 80);
    lv_obj_set_size(g_lv_ui_edit->spinbox, 180, 40);

    // 初始化数值：LVGL Spinbox 是整数，需要将 double * 10^precision
    lv_spinbox_set_value(g_lv_ui_edit->spinbox, 0);

    // --- 5. 单位 ---
    g_lv_ui_edit->edit_unit = lv_label_create(g_lv_ui_total->list);
    // lv_label_set_text(g_lv_ui_edit->edit_unit, param->unit);
    lv_obj_set_style_text_color(g_lv_ui_edit->edit_unit, lv_color_white(), 0);
    lv_obj_set_pos(g_lv_ui_edit->edit_unit, 190, 80);
    lv_obj_set_size(g_lv_ui_edit->edit_unit, 40, 100);

}
void ui_edit_open(Parameter* param)
{
    lv_label_set_text(g_lv_ui_edit->edit_title, param->name);
    // --- 4. 核心组件 Spinbox ---
    lv_spinbox_set_digit_format(g_lv_ui_edit->spinbox, param->digits, param->digits - param->precision);
    lv_spinbox_set_range(g_lv_ui_edit->spinbox, -99999999, 99999999);
    lv_spinbox_set_step(g_lv_ui_edit->spinbox, 1); // 这里的 1 是关键
    // 初始化数值：LVGL Spinbox 是整数，需要将 double * 10^precision
    int32_t int_val = (int32_t)(*param->value * get_scale_factor(param->precision));
    lv_spinbox_set_value(g_lv_ui_edit->spinbox, int_val);
    // --- 5. 单位 ---
    lv_label_set_text(g_lv_ui_edit->edit_unit, param->unit);
}
void ui_display_set_value(char* instance_val,char* label_instance_unit,
	                    char* label_total_val,char* label_total_unit,char * sq,char * flag)
{
    char buf[32];
    char flag_1[10];
    // 1. 更新瞬时流量
    lv_label_set_text(g_lv_ui_display->label_instance_val, instance_val);

    lv_label_set_text(g_lv_ui_display->label_instance_unit, label_instance_unit);
    
    // 2. 更新圆环进度 (假设最大量程为 100.0)
    double val = strtod(instance_val, NULL);
    switch (g_parameters.flow_rate_unit) {
        case 0: // m3 或 m3/h
            val = val;
            break;
        case 1: // L 或 L/min
            val = val / M3H_TO_LMIN; // m3/h -> L/min
            break;
        case 2: // mL 或 mL/min
            val = val / M3H_TO_MLMIN; // m3/h -> mL/min
            break;
        default:
            val = val;
    }
    int arc_val = (int)((val/
    ((g_parameters.outer_diameter-2 * g_parameters.wall_thickmess)*
    (g_parameters.outer_diameter-2 * g_parameters.wall_thickmess) *3.14) * 20)*3600/4); 
    if(arc_val > 100) arc_val = 100;
    if(arc_val < 0) arc_val = 0;
    lv_arc_set_value(g_lv_ui_display->arc_flow, arc_val);

    if(0)
    {
        static bool blink = 0;
        if(blink)
        {
            lv_obj_set_style_arc_color(g_lv_ui_display->arc_flow, lv_color_hex(0xff0033), LV_PART_MAIN); // 轨道鲜红
            lv_obj_set_style_arc_color(g_lv_ui_display->arc_flow, lv_color_hex(0xff0033), LV_PART_INDICATOR); // 进度鲜红
            blink = !blink;
        }
        else
        {
            lv_obj_set_style_arc_color(g_lv_ui_display->arc_flow, lv_color_hex(0x000000), LV_PART_MAIN); // 轨道鲜红
            lv_obj_set_style_arc_color(g_lv_ui_display->arc_flow, lv_color_hex(0x000000), LV_PART_INDICATOR); // 进度鲜红
            blink = !blink;
        }

    }
    else
    {
        lv_obj_set_style_arc_color(g_lv_ui_display->arc_flow, lv_color_hex(0x333333), LV_PART_MAIN); // 轨道深灰
        lv_obj_set_style_arc_color(g_lv_ui_display->arc_flow, lv_color_hex(0xFF0000), LV_PART_INDICATOR); // 进度亮青
    }

    // 3. 更新累计流量
    lv_label_set_text(g_lv_ui_display->label_total_val, label_total_val);

    lv_label_set_text(g_lv_ui_display->label_total_unit, label_total_unit);

    // 4. 更新SQ
    sprintf(buf, "SQ: %s", sq);
    lv_label_set_text(g_lv_ui_display->label_sq, buf);
    sprintf(flag_1, "%s", flag);
    lv_label_set_text(g_lv_ui_display->label_flag, flag);
}


void ui_menu_all_disappear(void)
{
    lv_obj_set_size(g_lv_ui_menu->menu_list_title, 0, 0);
    lv_obj_set_size(g_lv_ui_menu->menu_list_page, 0, 0);
    lv_obj_set_size(g_lv_ui_menu->menu_list_total, 0, 0);
}
void ui_menu_all_appear(void)
{
    lv_obj_set_size(g_lv_ui_menu->menu_list_title, 140, 60);
    lv_obj_set_size(g_lv_ui_menu->menu_list_page, 60, 60);
    lv_obj_set_size(g_lv_ui_menu->menu_list_total, 240, 215);
}


void ui_display_all_disappear(void)
{
    lv_obj_set_size(g_lv_ui_display->arc_flow, 0, 0);
    lv_obj_set_size(g_lv_ui_display->label_sq, 0, 0);
    lv_obj_set_size(g_lv_ui_display->label_flag, 0, 0);
    lv_obj_set_size(g_lv_ui_display->label_total_val, 0, 0);
    lv_obj_set_size(g_lv_ui_display->label_total_unit, 0, 0);
}
void ui_display_all_appear(void)
{
    lv_obj_set_size(g_lv_ui_display->arc_flow, 190, 190);
    lv_obj_set_size(g_lv_ui_display->label_sq, 70, 15);
    lv_obj_set_size(g_lv_ui_display->label_flag, 70, 15);
    lv_obj_set_size(g_lv_ui_display->label_total_val, 70, 30);
    lv_obj_set_size(g_lv_ui_display->label_total_unit, 40, 30);
}

void ui_edit_all_disappear(void)
{
    lv_obj_set_size(g_lv_ui_edit->edit_title, 0, 0);
    lv_obj_set_size(g_lv_ui_edit->spinbox, 0, 0);
    lv_obj_set_size(g_lv_ui_edit->edit_unit, 0, 0);
}
void ui_edit_all_appear(void)
{
    lv_obj_set_size(g_lv_ui_edit->edit_title, 200, 60);
    lv_obj_set_size(g_lv_ui_edit->spinbox, 180, 40);
    lv_obj_set_size(g_lv_ui_edit->edit_unit, 40, 100);
}