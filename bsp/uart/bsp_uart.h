#ifndef STANDARD_ROBOT_BSP_UART_H
#define STANDARD_ROBOT_BSP_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#include "typedef.h"
#include "usart.h"
#include <functional>

#define BUFLEN 64
using UART_DecodeFunc = std::function<void(uint8_t*)>;

class UART_Instance
{
private:
    UART_HandleTypeDef *huart;
    uint8_t id;

protected:
    UART_DecodeFunc decode;

public:
    UART_Instance(UART_HandleTypeDef *huart, UART_DecodeFunc decode);
    ~UART_Instance();
    void cb_register() const;
    void cb_unregister() const;
    void send(const uint8_t* data, uint16_t len) const;
};

void uart_init(const UART_HandleTypeDef *huart, uint8_t double_buffer);
void usart_rec_handler(const UART_HandleTypeDef* huart);



#endif //STANDARD_ROBOT_BSP_UART_H