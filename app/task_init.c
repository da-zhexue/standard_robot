/* @file task_init.c
 * @brief 初始化FreeRTOS任务
 * @version 1.0
 */

#include "task_init.h"
#include "cmsis_os.h"
extern void CommTask(void const * argument);
extern void test_task(void const * argument);
extern void UlogTask(void const *argument);
osThreadId commTaskHandle;
osThreadId testTaskHandle;
osThreadId ulogTaskHandle;
void task_init()
{
    osThreadDef(commTask, CommTask, osPriorityNormal, 0, 128);
    commTaskHandle = osThreadCreate(osThread(commTask), NULL);
    osThreadDef(testTask, test_task, osPriorityNormal, 0, 128);
    testTaskHandle = osThreadCreate(osThread(testTask), NULL);
    osThreadDef(ulogTask, UlogTask, osPriorityNormal, 0, 256);
    ulogTaskHandle = osThreadCreate(osThread(ulogTask), NULL);
}