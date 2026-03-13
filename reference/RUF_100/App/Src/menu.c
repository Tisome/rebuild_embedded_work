// #include "oled.h" 
#include "menu.h"
#include "FreeRTOS.h"
#include "bsp_key.h"
#include "st7789_reg_define.h"
#include "bsp_st7789_driver.h"
#include "elog.h"
#include "lv_ui.h"
#include "data.h"

#define MENU_IS_SELECTED 1
#define MENU_IS_NOT_SELECTED 0

uint8_t start_pip = 0;//启动界面修正

extern bsp_st7789_driver_t st7789_driver_instance;
static void show_menu(Menu* menu, uint8_t selected_index) 
{
    ui_menu_list_set((char*)menu->title,g_parameters.language);
    menu->page = (selected_index / menu->items_per_page) ;
    uint8_t start_index = menu->page * menu->items_per_page;
    uint8_t end_index = start_index + menu->items_per_page;
    
    uint8_t display_index = 0;
    for (uint8_t i = start_index; i < end_index; i++) {
        if (i == selected_index) 
        {
            if(g_parameters.language)
            ui_menu_list_item_set(i%3,(char*)menu->items[i].text_cn,MENU_IS_SELECTED,1);
            else
            ui_menu_list_item_set(i%3,(char*)menu->items[i].text,MENU_IS_SELECTED,0);
        } else {
        if(i < menu->items_count)
        {
            if(g_parameters.language)
            ui_menu_list_item_set(i%3,(char*)menu->items[i].text_cn,MENU_IS_NOT_SELECTED,1);
            else
            ui_menu_list_item_set(i%3,(char*)menu->items[i].text,MENU_IS_NOT_SELECTED,0);
        }
        else
        {
            ui_menu_list_item_set_disappear(i%3);
        }
        }
        display_index++;
    }
    
    // 显示页码（如果有多页）
    uint8_t total_pages = (menu->items_count + menu->items_per_page - 1) / menu->items_per_page;
    char page_str[16];
    snprintf(page_str, sizeof(page_str), "<%d/%d>", menu->page + 1, total_pages);
    ui_menu_list_item_page_set(page_str);
    
}

static int handle_menu_selection(Menu* menu) {
    uint8_t selected_index = menu->start;
    uint8_t items_count = menu->items_count;
    
    while (1) {
        //首次上电跳转主显示界面
        if(0 == start_pip)
        {
            start_pip ++;
            menu->items[0].action(&selected_index);
        }
        show_menu(menu, selected_index);
        static int KeyNum;
        KeyNum = key_scan(1);
        switch (KeyNum) {
            case KEY0_PRES: 
                if (selected_index > 0) {
                    selected_index--;
                    // 如果当前项是当前页的第一项，且前面还有项，则翻页
                    if (selected_index < menu->page * menu->items_per_page) {
                        menu->page = (menu->page > 0) ? menu->page - 1 : 0;
                    }
                } else {

                    selected_index = items_count - 1;
                    menu->page = (items_count - 1) / menu->items_per_page;
                }
                break;
                
            case KEY1_PRES: 
                if (selected_index < items_count - 1) {
                    selected_index++;
                    if (selected_index >= (menu->page + 1) * menu->items_per_page) {
                        menu->page++;
                    }
                } else {

                    selected_index = 0;
                    menu->page = 0;
                }
                break;
            
            case KEY2_PRES: //
                if (menu->items[selected_index].action) {
                    if (menu->type != 1) 
                    {   
                        menu->items[selected_index].action(&selected_index);
                        break;
                    }
                    else
                    {
                        return menu->items[selected_index].action(&selected_index); 
                    }                   
                }
                break;
                
            case KEY3_PRES: 
                return 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}



int display_menu(Menu* menu) {
    menu->page = 0; // 重置到第一页
    return handle_menu_selection(menu);
}

void display_static_menu_item(Menu* menu)
{
    show_menu(menu, menu->start);
}





