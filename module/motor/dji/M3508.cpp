#include "M3508.h"

M3508::M3508(CAN_HandleTypeDef* handler, const uint32_t tx_id, const uint8_t motor_num)
    : CANInstance(handler, tx_id, 0, CAN_ID_STD, 8, CAN_RTR_DATA, nullptr)
{
    this->motor_num = (motor_num > 4) ? 4 : motor_num;
    if(tx_id == 0x200) {
        for(int i = 0; i < motor_num; i++) {
            rx_ids[i] = 0x201 + i;
        }
    }
    else if(tx_id == 0x1FF) {
        for(int i = 0; i < motor_num; i++) {
            rx_ids[i] = 0x205 + i;
        }
    }
    else{
        for(int i = 0; i < motor_num; i++) {
            rx_ids[i] = 0;
        }
    }
    for(int i = 0; i < motor_num; i++)
    {
        auto decode_func = [this, i](uint8_t* data) {
                this->decode(data, i);
        };
        cb_register(rx_ids[i], decode_func);
    }
}
M3508::~M3508()
{
    for (int i = 0; i < motor_num; i++) {
        cb_unregister(rx_ids[i]);
    }
}

void M3508::send_cmd(const int16_t motor1, const int16_t motor2, const int16_t motor3, const int16_t motor4)
{
    uint8_t tx_data[8];
    tx_data[0] = motor1 >> 8;
    tx_data[1] = motor1;
    tx_data[2] = motor2 >> 8;
    tx_data[3] = motor2;
    tx_data[4] = motor3 >> 8;
    tx_data[5] = motor3;
    tx_data[6] = motor4 >> 8;
    tx_data[7] = motor4;

    send(tx_data);
}

void M3508::decode(const uint8_t* data, const uint8_t motor)
{
    m3508_measure[motor].last_ecd = static_cast<int16_t>(m3508_measure[motor].ecd);
    m3508_measure[motor].ecd = static_cast<uint16_t>(data[0] << 8 | data[1]);
    m3508_measure[motor].speed = static_cast<int16_t>(data[2] << 8 | data[3]);
    m3508_measure[motor].current = static_cast<int16_t>(data[4] << 8 | data[5]);
    m3508_measure[motor].temperature = data[6];
    //此处发布话题
}
