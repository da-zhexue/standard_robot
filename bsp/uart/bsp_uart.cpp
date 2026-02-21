/* @file bsp_uart.cpp
 * @brief USART驱动
 * @version 1.0
 * @TODO: 增加错误日志。如何区分不同设备？(目前中断回调会进入同一个usart的所有decode函数)
 * @TODO: bsp层应该返回错误码，以便module层log输出，而不是在bsp层直接log输出，bsp层程序必须保持独立性，减少对其他文件的依赖，以便于移植
 * @TODO: 需要修改不采用双缓冲区时的串口初始化与中断回调方式，当前方式需要在cubemx中将接收方式从Circle改为Normal
 */

#include "uart/bsp_uart.h"
#define GET_UART_INDEX(instance) ((instance) == USART1 ? 0 : ((instance) == USART3 ? 1 : 2))

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart6_rx;

static uint8_t uart1_rx_buf[2][BUFLEN];
static uint8_t uart3_rx_buf[2][BUFLEN];
static uint8_t uart6_rx_buf[2][BUFLEN];

static uint16_t uart_id[3] = {0}; // 0: USART1, 1: USART3, 2: USART6
static UART_DecodeFunc uart_map[3][5] = {}; // 0: USART1, 1: USART3, 2: USART6

UART_Instance::UART_Instance(UART_HandleTypeDef *huart, UART_DecodeFunc decode) : huart(huart), decode(std::move(decode))
{
    if (huart->Instance == USART1) __CLEAR(uart1_rx_buf);
    else if (huart->Instance == USART6) __CLEAR(uart6_rx_buf);
    const uint8_t uart_ins = GET_UART_INDEX(huart->Instance);
    id = uart_id[uart_ins]++;
    cb_register();
}
UART_Instance::~UART_Instance()
{
    cb_unregister();
}

void UART_Instance::cb_register() const
{
    const auto index = GET_UART_INDEX(huart->Instance);
    uart_map[index][id] = decode;
}
void UART_Instance::cb_unregister() const
{
    const auto index = GET_UART_INDEX(huart->Instance);
    uart_map[index][id] = nullptr;
}

void UART_Instance::send(const uint8_t* data, const uint16_t len) const
{
    HAL_UART_Transmit_DMA(huart, data, len);
}

void cb_handle(const USART_TypeDef* instance, uint8_t* data)
{
    const auto index = GET_UART_INDEX(instance);
    for (uint8_t i = 0; i < uart_id[index]; ++i)
    {
        if (uart_map[index][i])
        {
            uart_map[index][i](data);
        }
    }
}

extern "C"{

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) //DMA
{
    if(huart == &huart1)
    {
#ifndef USART1_DOUBLE_BUFFER_ENABLE
        cb_handle(USART1, uart1_rx_buf[0]);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_buf[0], sizeof(uart1_rx_buf[0]));
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
#endif
    }
    else if(huart == &huart6)
    {
#ifndef USART6_DOUBLE_BUFFER_ENABLE
        cb_handle(USART6, uart6_rx_buf[0]);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart6, uart6_rx_buf[0], sizeof(uart6_rx_buf[0]));
        __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
#endif
    }
}

}
void uart_init(const UART_HandleTypeDef *huart, const uint8_t double_buffer)
{
    if (double_buffer)
    {
        const auto hdma_rx = huart->hdmarx;
        const auto dma_instance = huart->hdmarx->Instance;
        uint8_t* buf0 = (huart->Instance == USART1 ? uart1_rx_buf[0] : (huart->Instance == USART6 ? uart6_rx_buf[0] : uart3_rx_buf[0]));
        uint8_t* buf1 = (huart->Instance == USART1 ? uart1_rx_buf[1] : (huart->Instance == USART6 ? uart6_rx_buf[1] : uart3_rx_buf[1]));
        SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
        __HAL_DMA_DISABLE(hdma_rx);
        while (dma_instance->CR & DMA_SxCR_EN)
        {
            __HAL_DMA_DISABLE(hdma_rx);
        }
        dma_instance->PAR = reinterpret_cast<uint32_t>(&(huart->Instance->DR));
        dma_instance->M0AR = reinterpret_cast<uint32_t>(buf0);
        dma_instance->M1AR = reinterpret_cast<uint32_t>(buf1);
        dma_instance->NDTR = BUFLEN;
        SET_BIT(dma_instance->CR, DMA_SxCR_DBM);
        __HAL_DMA_ENABLE(hdma_rx);
    }
    else
    {
        if (huart->Instance == USART1)
        {
            HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_buf[0], sizeof(uart1_rx_buf[0]));
            __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
        }
        else if (huart->Instance == USART3)
        {
            HAL_UARTEx_ReceiveToIdle_DMA(&huart3, uart3_rx_buf[0], sizeof(uart3_rx_buf[0]));
            __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
        }
        else if (huart->Instance == USART6)
        {
            HAL_UARTEx_ReceiveToIdle_DMA(&huart6, uart6_rx_buf[0], sizeof(uart6_rx_buf[0]));
            __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
        }
    }
}

void usart_rec_handler(const UART_HandleTypeDef* huart)
{
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE))
    {
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
    else if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
    {
        static uint16_t this_time_rx_len = 0;
        __HAL_UART_CLEAR_PEFLAG(huart);
        const auto *hdma_rx = huart->hdmarx;
        if ((hdma_rx->Instance->CR & DMA_SxCR_CT) == RESET)
        {
            __HAL_DMA_DISABLE(hdma_rx);
            this_time_rx_len = BUFLEN - hdma_rx->Instance->NDTR;
            hdma_rx->Instance->NDTR = BUFLEN;
            hdma_rx->Instance->CR |= DMA_SxCR_CT;
            __HAL_DMA_ENABLE(hdma_rx);
            if(this_time_rx_len > 0)
            {
                uint8_t* buf0 = (huart->Instance == USART1 ? uart1_rx_buf[0] : (huart->Instance == USART6 ? uart6_rx_buf[0] : uart3_rx_buf[0]));
                cb_handle(huart->Instance, buf0); //
            }
        }
        else
        {
            __HAL_DMA_DISABLE(hdma_rx);
            this_time_rx_len = BUFLEN - hdma_rx->Instance->NDTR;
            hdma_rx->Instance->NDTR = BUFLEN;
            hdma_rx->Instance->CR &= ~(DMA_SxCR_CT);
            __HAL_DMA_ENABLE(hdma_rx);

            if(this_time_rx_len > 0)
            {
                uint8_t* buf1 = (huart->Instance == USART1 ? uart1_rx_buf[1] : (huart->Instance == USART6 ? uart6_rx_buf[1] : uart3_rx_buf[1]));
                cb_handle(huart->Instance, buf1);
            }
        }
    }
}



