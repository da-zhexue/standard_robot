#ifndef M3508_H
#define M3508_H

#include "typedef.h"
#include "can.h"
#include "bsp_can.h"

class M3508 : public CANInstance
{
private:
    typedef struct
    {
        uint16_t ecd;
        int16_t speed;
        int16_t current;
        int16_t temperature;
        int16_t last_ecd;
    }m3508_t;

    uint32_t rx_ids[4];
    uint8_t motor_num;
    m3508_t m3508_measure[4];
public:
    M3508(CAN_HandleTypeDef* handler, uint32_t tx_id, uint8_t motor_num);
    ~M3508();

    void send_cmd(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4);
    void decode(const uint8_t* data, const uint8_t motor);
};

#endif // M3508_H