#ifndef _LV_UI_H
#define _LV_UI_H

#include "main.h"
#include "menu.h"
#include "lv_ui.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "parameter_editor.h"

typedef struct
{
  	lv_obj_t *screen;
	lv_obj_t *list;
}lv_ui_total;

extern lv_ui_total* g_lv_ui_total;

void ui_menu_total_init(void);

typedef struct
{
	lv_obj_t *menu_list_title;
	lv_obj_t *menu_list_page;
	lv_obj_t *menu_list_total;
	lv_obj_t *menu_list_cont[DEFAULT_ITEMS_PER_PAGE];
	lv_obj_t *menu_label[DEFAULT_ITEMS_PER_PAGE];
}lv_ui_menu;

extern lv_ui_menu* g_lv_ui_menu;

void ui_menu_list_init(void);
void ui_menu_list_set(char** title,bool language);
void ui_menu_list_item_page_set(char* index);
void ui_menu_list_item_set(uint32_t index,char* text,bool is_select,bool languange);	
void ui_menu_list_item_set_disappear(uint32_t index);

void ui_menu_all_disappear(void);
void ui_menu_all_appear(void);

typedef struct
{
	lv_obj_t * arc_flow;
	lv_obj_t * label_instance_val;
	lv_obj_t * label_instance_unit;
	lv_obj_t * label_total_val;
	lv_obj_t * label_total_unit;

	lv_obj_t * label_sq;
	lv_obj_t * label_flag;
}lv_ui_display; 

extern lv_ui_display* g_lv_ui_display;

void ui_diplay_init(void);
void ui_display_set_value(char* instance_val,char* label_instance_unit,
	                    char* label_total_val,char* label_total_unit,char * sq,char * flag);

void ui_display_all_disappear(void);
void ui_display_all_appear(void);

typedef struct
{
	lv_obj_t * edit_title;
	lv_obj_t * spinbox;   
	lv_obj_t * edit_unit;   
}lv_ui_edit; 

extern lv_ui_edit* g_lv_ui_edit;

void ui_edit_init();
void ui_edit_open(Parameter* param);


void ui_edit_all_disappear(void);
void ui_edit_all_appear(void);
#endif



