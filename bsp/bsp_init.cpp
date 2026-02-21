#include "bsp_init.h"
#include "can/bsp_can.h"
#include "uart/bsp_uart.h"
#include "dtm/dtm.h"
#include "dwt/bsp_dwt.h"
#include "online_detect/onl_det.h"

extern "C"
{

float get_current_time() {
    return DWT_GetTimeline_s();  // 使用原来的时间函数
}

void bsp_init()
{

    // 初始化dtm数据中转站与OD在线状态监控器
    dtm::Manager::init();
    OD::init(get_current_time);
    // 初始化CAN滤波器
    can_filter_init(&hcan1);
    can_filter_init(&hcan2);

    // 初始化UART接收
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
