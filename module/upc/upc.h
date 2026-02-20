#ifndef STANDARD_ROBOT_UPC_H
#define STANDARD_ROBOT_UPC_H
#include "bsp_uart.h"
#include "bsp_can.h"

#define UPC_HEADER 0xA5
#define UPC_HEADER_LEN 5
#define UPC_DATA_LEN 0x0D
#define UPC_TOTAL_LEN (UPC_DATA_LEN + 9)

class upc : public UART_Instance
{
private:
    typedef struct
    {
        uint8_t start_upc_flag;
        uint8_t mode;
        uint8_t shoot;

        float vx, vy, vw;
        float gimbal_yaw, chassis_yaw;
        float small_gimbal_yaw, small_gimbal_pitch;
        float x, y, z;

    } upc_t;
    typedef enum
    {
        SEND_ATTITUDE = 0x301,
        SNED_REFEREE = 0x302,

        CMD_IMU_INFO = 0x0401, // 弃用
        CMD_MOVE = 0x0402,
        CMD_GIMBAL_ROTATION = 0x0403,
        CMD_SHOOT = 0x0404,
        CMD_MODE_SWITCH = 0x405,

        CMD_IMU_S_INFO = 0x101,
        CMD_IMU_L_INFO = 0x102
    } upc_cmd_t;

    upc_t upc_data{};
    CANInstance can_instance;
    void cmd_move_handler(const uint8_t* data);
    void cmd_gimbal_handler(const uint8_t* data);
    void cmd_shoot_handler(const uint8_t* data);
    void cmd_mode_handler(const uint8_t* data);
    void cmd_imu_s_handler(const uint8_t* data);
    void cmd_imu_l_handler(const uint8_t* data);
public:
    explicit upc(UART_HandleTypeDef *huart);
    ~upc();
    void decode(uint8_t* data);
    void send_attitude_handler() const;
    void enable();
    void disable();

};


#endif //STANDARD_ROBOT_UPC_H