#include "app_config.h"
#include "data.h"
#include "freertos_resources.h"
#include "task_manager.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "sys.h"

#include "elog.h"
#define LOG_TAG "key_task"
#define LOG_LVL ELOG_LVL_VERBOSE

static TaskHandle_t task_key_handle = NULL;
