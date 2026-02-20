#include "bsp_init.h"
#include "bsp_can.h"
#include "bsp_uart.h"

extern "C"
{

void bsp_init()
{
    // 1. 初始化CAN滤波器
    can_filter_init(&hcan1);
    can_filter_init(&hcan2);

    // 2. 初始化UART接收
#ifdef USART1_DOUBLE_BUFFER_ENABLE
    uart_init(&huart1, 1);
#else
    uart_init(&huart1, 0);
#endif
#ifdef USART3_DOUBLE_BUFFER_ENABLE
    uart_init(&huart3, 1);
#else
    uart_init(&huart3, 0);
#endif
#ifdef USART6_DOUBLE_BUFFER_ENABLE
    uart_init(&huart6, 1);
#else
    uart_init(&huart6, 0);
#endif

}

}
