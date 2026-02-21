#ifndef STANDARD_ROBOT_ONL_DET_H
#define STANDARD_ROBOT_ONL_DET_H
#include "typedef.h"
#include <cstdint>
#include <cstring>

#ifndef OD_MAX_DEVICES
#define OD_MAX_DEVICES 32
#endif

class OD {
private:
    struct DeviceInfo {
        char name[32]{};
        uint32_t id;
        float last_online_time;
        bool is_registered;

        DeviceInfo();
    };

    static DeviceInfo devices_[OD_MAX_DEVICES];
    static uint32_t device_count_;
    static uint32_t online_bitmap_;
    static float (*get_time_func_)();

public:
    // 禁止实例化
    OD() = delete;
    OD(const OD&) = delete;
    OD& operator=(const OD&) = delete;
    ~OD() = delete;

    static void init(float (*get_time_func)());
    static int32_t register_device(const char* name, uint32_t id = 0);
    static int32_t find_device(const char* name);
    static void update(int32_t handle);
    static bool update_by_name(const char* name);
    static bool detect(int32_t handle, float timeout = 1.0f);
    static bool detect_by_name(const char* name, float timeout = 1.0f);
    static bool is_online(int32_t handle);
    static uint32_t get_device_count();
    static uint32_t get_online_bitmap();
};

#endif //STANDARD_ROBOT_ONL_DET_H