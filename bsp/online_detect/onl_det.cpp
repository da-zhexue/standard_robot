#include "onl_det.h"

OD::DeviceInfo OD::devices_[OD_MAX_DEVICES];
uint32_t OD::device_count_ = 0;
uint32_t OD::online_bitmap_ = 0;
float (*OD::get_time_func_)() = nullptr;

OD::DeviceInfo::DeviceInfo()
    : id(0), last_online_time(0.0f), is_registered(false) {
    name[0] = '\0';
}

void OD::init(float (*get_time_func)()) {
    get_time_func_ = get_time_func;
    device_count_ = 0;
    online_bitmap_ = 0;

    for (auto & device : devices_) {
        device = DeviceInfo();
    }
}

int32_t OD::register_device(const char* name, const uint32_t id) {
    if (device_count_ >= OD_MAX_DEVICES) {
        return -1;  // 达到设备数量上限
    }

    if (!name || strlen(name) == 0) {
        return -1;  // 设备名称为空
    }

    // 检查是否已存在同名设备
    for (uint32_t i = 0; i < device_count_; i++) {
        if (devices_[i].is_registered && strcmp(devices_[i].name, name) == 0) {
            return -1;  // 设备已存在
        }
    }

    const uint32_t index = device_count_;
    device_count_++;

    strncpy(devices_[index].name, name, sizeof(devices_[index].name) - 1);
    devices_[index].name[sizeof(devices_[index].name) - 1] = '\0';
    devices_[index].id = (id == 0) ? index : id;
    devices_[index].last_online_time = get_time_func_();
    devices_[index].is_registered = true;

    online_bitmap_ |= (1 << index);
    return static_cast<int32_t>(index);
}

int32_t OD::find_device(const char* name) {
    if (!name) return -1;

    for (uint32_t i = 0; i < device_count_; i++) {
        if (devices_[i].is_registered && strcmp(devices_[i].name, name) == 0) {
            return static_cast<int32_t>(i);
        }
    }
    return -1;
}

void OD::update(const int32_t handle) {
    if (handle < 0 || handle >= static_cast<int32_t>(device_count_) ||
        !devices_[handle].is_registered) {
        return;
        }

    devices_[handle].last_online_time = get_time_func_();
    online_bitmap_ |= (1 << handle);
}

bool OD::update_by_name(const char* name) {
    const int32_t handle = find_device(name);
    if (handle >= 0) {
        update(handle);
        return true;
    }
    return false;
}

bool OD::detect(const int32_t handle, const float timeout) {
    if (handle < 0 || handle >= static_cast<int32_t>(device_count_) ||
        !devices_[handle].is_registered) {
        return false;
        }

    const float current_time = get_time_func_();
    if (current_time - devices_[handle].last_online_time > timeout) {
        online_bitmap_ &= ~(1 << handle);
        return false;
    }

    online_bitmap_ |= (1 << handle);
    return true;
}

bool OD::detect_by_name(const char* name, const float timeout) {
    const int32_t handle = find_device(name);
    if (handle >= 0) {
        return detect(handle, timeout);
    }
    return false;
}

bool OD::is_online(const int32_t handle) {
    if (handle < 0 || handle >= static_cast<int32_t>(device_count_)) {
        return false;
    }
    return (online_bitmap_ & (1 << handle)) != 0;
}

uint32_t OD::get_device_count() { return device_count_; }
uint32_t OD::get_online_bitmap() { return online_bitmap_; }
