#ifndef BSP_CAN_H
#define BSP_CAN_H
#include "typedef.h"
#include "can.h"
#include <functional>

using CAN_DecodeFunc = std::function<void(uint8_t*)>;

class CANInstance
{
private:
    CAN_HandleTypeDef* handler{};
    uint32_t send_mailbox;
    uint32_t rx_id;
    uint8_t rx_data[8]{};
    CAN_TxHeaderTypeDef can_txheader{};

protected:
    CAN_DecodeFunc decode;

public:
    CANInstance(CAN_HandleTypeDef* handler, uint32_t tx_id, uint32_t rx_id, uint32_t IDE, uint32_t DLC, uint32_t RTR, CAN_DecodeFunc decode = nullptr);
    ~CANInstance();
    void cb_register(uint32_t id, const CAN_DecodeFunc &decode_func);
    void cb_register();
    void cb_unregister(uint32_t id) const;
    void send(const uint8_t* tx_data);

};
void can_filter_init(CAN_HandleTypeDef* hcan);
#endif //BSP_CAN_H