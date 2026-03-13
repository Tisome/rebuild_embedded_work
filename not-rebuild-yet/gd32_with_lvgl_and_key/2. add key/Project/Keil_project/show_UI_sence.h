#include "gd32f30x.h"


typedef enum
{
    UI_SCENE_MAIN = 0,
    UI_SCENE_MENU,
    UI_SCENE_DN_PARAM_EDIT,
    UI_SCENE_METERIAL_PARAM_EDIT
} ui_scene_t;

static ui_scene_t g_ui_scene = UI_SCENE_MAIN;
static uint8_t g_menu_index = 0;