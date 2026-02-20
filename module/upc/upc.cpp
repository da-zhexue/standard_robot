#include "upc.h"
#include "crc.h"
#include "user_lib.h"
#include <functional>
#include "dtm.h"

DTM_DECLARE_TOPIC(float, test1);

upc::upc(UART_HandleTypeDef *huart)
    : UART_Instance(huart, [this]<typename T0>(T0 && PH1) { decode(std::forward<T0>(PH1)); }),
      can_instance(&hcan2, 0x223, 0, CAN_ID_STD, 8, CAN_RTR_DATA, nullptr)
{
    upc_data.start_upc_flag = 0;
}

upc::~upc() = default;

void upc::enable()
{
    upc_data.start_upc_flag = 1;
}
void upc::disable()
{
    upc_data.start_upc_flag = 0;
}

void upc::send_attitude_handler() const
{
    static uint8_t send_data[UPC_TOTAL_LEN] = {0};
    send_data[0] = UPC_HEADER;
    send_data[1] = UPC_DATA_LEN;
    send_data[2] = 0;send_data[3] = 0;
    send_data[5] = SEND_ATTITUDE & 0xFF;
    send_data[6] = (SEND_ATTITUDE >> 8) & 0xFF;

    float temp = 0.0f;
    DTM_GET(test1, temp);
    pack_float_to_4bytes(temp, &send_data[7]);
    // pack_float_to_4bytes(tf_ptr->Chassis_angle.yaw_deg, &send_data[7]);
    // pack_float_to_4bytes(tf_ptr->Small_Gimbal_angle.yaw_deg, &send_data[11]);
    // pack_float_to_4bytes(tf_ptr->Small_Gimbal_angle.pitch_deg, &send_data[15]);

    Append_CRC8_Check_Sum(send_data, UPC_HEADER_LEN);
    Append_CRC16_Check_Sum(send_data, UPC_TOTAL_LEN);
    send(send_data, sizeof(send_data));
}

void upc::decode(uint8_t* data)
{
    if(!upc_data.start_upc_flag)
        return ;
    if(data[0] != UPC_HEADER || data[2] != 0 || data[3] != 0)
        return ;
    if(data[1] != UPC_DATA_LEN)
        return ;
    if(!Verify_CRC8_Check_Sum(data, UPC_HEADER_LEN) || !Verify_CRC16_Check_Sum(data , UPC_TOTAL_LEN))
        return ;

    const uint16_t cmd_id = (data[6] << 8) | data[5];

    switch(cmd_id)
    {
    case CMD_IMU_INFO:
        break;
    case CMD_MOVE:
        cmd_move_handler(&data[UPC_HEADER_LEN+2]);
        break;
    case CMD_GIMBAL_ROTATION:
        cmd_gimbal_handler(&data[UPC_HEADER_LEN+2]);
        break;
    case CMD_SHOOT:
        cmd_shoot_handler(&data[UPC_HEADER_LEN+2]);
        break;
    case CMD_MODE_SWITCH:
        cmd_mode_handler(&data[UPC_HEADER_LEN+2]);
        break;
    case CMD_IMU_S_INFO:
        //cmd_imu_s_handler(&data[UPC_HEADER_LEN+2]);
        break;
    case CMD_IMU_L_INFO:
        cmd_imu_l_handler(&data[UPC_HEADER_LEN+2]);
        break;
    default:
        break;
    }
    //此处发布话题
    return ;
}

void upc::cmd_move_handler(const uint8_t* data)
{
    unpack_4bytes_to_floats(&data[0], &upc_data.vx);
    unpack_4bytes_to_floats(&data[4], &upc_data.vy);
    unpack_4bytes_to_floats(&data[8], &upc_data.vw);
    //OMM_update(UPC_ONLINE);
}

void upc::cmd_gimbal_handler(const uint8_t* data)
{
    unpack_4bytes_to_floats(&data[0], &upc_data.gimbal_yaw);
    unpack_4bytes_to_floats(&data[4], &upc_data.small_gimbal_yaw);
    unpack_4bytes_to_floats(&data[8], &upc_data.small_gimbal_pitch);
    //OMM_update(UPC_ONLINE);
}

void upc::cmd_shoot_handler(const uint8_t* data)
{
    uint8_t send_data[8] = {0};
    send_data[0] = data[12];

    can_instance.send(send_data);
    // OMM_update(UPC_ONLINE);
}

void upc::cmd_mode_handler(const uint8_t* data) // 暂时用于摩擦轮控制
{
    //upc.mode = data[12];
    uint8_t send_data[8] = {0};
    send_data[1] = data[12];
    can_instance.send(send_data);
    // OMM_update(UPC_ONLINE);
}

void upc::cmd_imu_s_handler(const uint8_t* data)
{
    if (sizeof(data) >= 10)
        return ;
    static fp32 gimbal_s_ptr[3];
    unpack_4bytes_to_floats(&data[0], &gimbal_s_ptr[0]);
    unpack_4bytes_to_floats(&data[4], &gimbal_s_ptr[1]);
    unpack_4bytes_to_floats(&data[8], &gimbal_s_ptr[2]);
    // DTM_Write(GIMBAL_S_DATA, gimbal_s_ptr, sizeof(gimbal_s_ptr));
    // OMM_update(GIMBAL_S_ONLINE);
}

void upc::cmd_imu_l_handler(const uint8_t* data)
{
    static fp32 gimbal_l_ptr[3];
    unpack_4bytes_to_floats(&data[0], &gimbal_l_ptr[0]);
    unpack_4bytes_to_floats(&data[4], &gimbal_l_ptr[1]);
    unpack_4bytes_to_floats(&data[8], &gimbal_l_ptr[2]);
    // DTM_Write(GIMBAL_L_DATA, gimbal_l_ptr, sizeof(gimbal_l_ptr));
    // OMM_update(GIMBAL_L_ONLINE);
}

