/* @file bsp_can.cpp
 * @brief CAN总线驱动
 * @version 1.0
 * @TODO : 增加错误日志。如果使用扩展ID，有些设备ID可能包含数据，如何处理？can_map需要改为链表形式方便增删。
 */

#include "bsp_can.h"
#include <map>

typedef struct CAN_Callback
{
    CAN_TypeDef* Instance{};
    uint32_t rx_id{};
    CAN_DecodeFunc decode;
} CAN_Callback;
static CAN_Callback can_map[50];
static uint16_t can_count = 0;

CANInstance::CANInstance(CAN_HandleTypeDef* handler, const uint32_t tx_id, const uint32_t rx_id,
    const uint32_t IDE = CAN_ID_STD, const uint32_t DLC = 8, const uint32_t RTR = CAN_RTR_DATA,
    CAN_DecodeFunc decode) : decode(std::move(decode))
{
    this->handler = handler;
    this->rx_id = rx_id;
    can_txheader.IDE = IDE;
    can_txheader.StdId = (IDE == CAN_ID_STD ? tx_id : 0);
    can_txheader.ExtId = (IDE == CAN_ID_EXT ? tx_id : 0);
    can_txheader.RTR = RTR;
    can_txheader.DLC = DLC;
    this->send_mailbox = 0;

    cb_register();
    __CLEAR(rx_data);
}
CANInstance::~CANInstance()
{
    cb_unregister(rx_id);
}

void CANInstance::cb_register(const uint32_t id, const CAN_DecodeFunc& decode_func)
{
    if (id == 0) return;
    for (int i = 0; i < can_count; ++i){
        if (can_map[i].Instance == handler->Instance && can_map[i].rx_id == id){
            can_map[i].decode = decode_func;
            return;
        }
    }
    can_map[can_count].Instance = handler->Instance;
    can_map[can_count].rx_id = id;
    can_map[can_count].decode = decode_func;
    can_count++;
}
void CANInstance::cb_register()
{
    if (rx_id == 0) return;
    for (int i = 0; i < can_count; ++i){
        if (can_map[i].Instance == handler->Instance && can_map[i].rx_id == rx_id){
            can_map[i].decode = decode;
            //@TODO: 错误日志：同一设备同一ID注册了多个回调函数，覆盖之前的回调函数
            return;
        }
    }
    can_map[can_count].Instance = handler->Instance;
    can_map[can_count].rx_id = rx_id;
    can_map[can_count].decode = decode;
    can_count++;
}
void CANInstance::cb_unregister(uint32_t id) const
{
    if (id == 0) return;
    for (int i = 0; i < can_count; ++i){
        if (can_map[i].Instance == handler->Instance && can_map[i].rx_id == id){
            can_map[i].decode = nullptr;
            return;
        }
    }
    //can_count--;
}

void CANInstance::send(const uint8_t* tx_data)
{
    HAL_CAN_AddTxMessage(handler, &can_txheader, tx_data, &send_mailbox);
}

void can_filter_init(CAN_HandleTypeDef* hcan)
{
    CAN_FilterTypeDef canfilter;

    canfilter.FilterBank = 0;
    canfilter.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilter.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilter.FilterIdHigh = 0x0000;
    canfilter.FilterIdLow = 0x0000;
    canfilter.FilterMaskIdHigh = 0x0000;
    canfilter.FilterMaskIdLow = 0x0000;
    canfilter.FilterFIFOAssignment = CAN_RX_FIFO0;
    canfilter.FilterActivation = ENABLE;
    canfilter.SlaveStartFilterBank = 13;

    HAL_CAN_ConfigFilter(hcan, &canfilter);
    HAL_CAN_Start(hcan);
}

void cb_handle(CAN_TypeDef* CANx, uint32_t RxId, uint8_t* data) // @TODO: 如何提高找到正确回调速度
{
    for (int i = 0; i < can_count; ++i){
        if (can_map[i].Instance == CANx && can_map[i].rx_id == RxId){
            if (can_map[i].decode)
                can_map[i].decode(data);
            return;
        }
    }
}
extern "C"{

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

    const uint32_t id = (rx_header.IDE == CAN_ID_STD) ? rx_header.StdId : rx_header.ExtId;
    cb_handle(hcan->Instance, id, rx_data);
}

}
