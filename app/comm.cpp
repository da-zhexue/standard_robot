#include "comm.h"
#include "upc.h"
#include "cmsis_os.h"
upc upc_instance(&huart6);

void comm_init()
{
    upc_instance.enable();
}

void CommTask(void const * argument)
{
    comm_init();
    while (1)
    {
        upc_instance.send_attitude_handler();
        osDelay(5);
    }
}