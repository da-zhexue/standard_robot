#pragma once

#include "dtm_type_traits.h"
#include <cstring>
#include <cstdint>
#include <string_view>

// 配置参数
#ifndef DTM_MAX_TOPICS
#define DTM_MAX_TOPICS 32
#endif

#ifndef DTM_MAX_NAME_LENGTH
#define DTM_MAX_NAME_LENGTH 64
#endif

namespace dtm {

/**
 * @brief 错误码定义
 */
enum class Error {
    SUCCESS = 0,
    TOPIC_NOT_FOUND,
    TOPIC_ALREADY_EXISTS,
    TYPE_MISMATCH,
    BUFFER_FULL,
    INVALID_PARAM
};

/**
 * @brief 话题信息结构体
 */
struct TopicInfo {
    const char* name;       // 话题名称
    void* data_ptr;         // 数据指针
    size_t data_size;       // 数据大小
    uint64_t type_id;       // 类型ID（替代typeid）
    bool registered;        // 是否已注册

    constexpr TopicInfo()
        : name(nullptr), data_ptr(nullptr), data_size(0),
          type_id(0), registered(false) {}

    constexpr TopicInfo(const char* n, void* ptr, const size_t sz, const uint64_t tid)
        : name(n), data_ptr(ptr), data_size(sz),
          type_id(tid), registered(true) {}
};

/**
 * @brief 话题数据存储模板类
 */
template<typename T>
class TopicStorage {
private:
    T data_;  // 静态数据存储

public:
    T& get() { return data_; }
    const T& get() const { return data_; }
    T* ptr() { return &data_; }
    const T* ptr() const { return &data_; }
    static constexpr size_t size() { return sizeof(T); }
    constexpr uint64_t type_id() {
        return type_traits::TypeID<T>::id();
    }
};

/**
 * @brief 数据中转管理器（完全静态，无RTTI）
 */
class Manager {
private:

    // 静态存储区
     static TopicInfo s_topics[DTM_MAX_TOPICS];
     static uint32_t s_topic_count;

    // 内部查找函数
     static TopicInfo* findTopic(const char* name);

public:
    // 禁止实例化
    Manager() = delete;
    ~Manager() = delete;
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    /**
     * @brief 初始化管理器
     */
    static void init();

    /**
     * @brief 注册话题
     * @tparam T 数据类型
     * @param name 话题名称（必须是字符串字面量）
     * @param storage 数据存储引用
     * @return 成功返回SUCCESS
     */
    template<typename T>
    static Error registerTopic(const char* name, TopicStorage<T>& storage);

    /**
     * @brief 发布数据
     * @tparam T 数据类型
     * @param name 话题名称
     * @param data 数据
     * @return 成功返回SUCCESS
     */
    template<typename T>
    static Error publish(const char* name, const T& data);

    /**
     * @brief 获取数据
     * @tparam T 数据类型
     * @param name 话题名称
     * @param data 输出数据
     * @return 成功返回SUCCESS
     */
    template<typename T>
    static Error get(const char* name, T& data);

    /**
     * @brief 获取数据指针
     * @tparam T 数据类型
     * @param name 话题名称
     * @return 数据指针，失败返回nullptr
     */
    template<typename T>
    static T* getPtr(const char* name);

    /**
     * @brief 检查话题是否存在
     * @param name 话题名称
     * @return 是否存在
     */
     static bool exists(const char* name);

    /**
     * @brief 获取话题数量
     */
     static uint32_t count() { return s_topic_count; }

    /**
     * @brief 清空所有话题
     */
     static void clear();
};

inline TopicInfo* Manager::findTopic(const char* name) {
    for (uint32_t i = 0; i < s_topic_count; ++i) {
        if (s_topics[i].registered &&
            std::strcmp(s_topics[i].name, name) == 0) {
            return &s_topics[i];
        }
    }
    return nullptr;
}

template<typename T>
Error Manager::registerTopic(const char* name, TopicStorage<T>& storage) {
    // 检查缓冲区是否已满
    if (s_topic_count >= DTM_MAX_TOPICS) {
        return Error::BUFFER_FULL;
    }

    // 检查是否已存在同名话题
    if (findTopic(name) != nullptr) {
        return Error::TOPIC_ALREADY_EXISTS;
    }

    // 注册话题
    s_topics[s_topic_count] = TopicInfo(
        name,
        storage.ptr(),
        storage.size(),
        storage.type_id()
    );

    ++s_topic_count;
    return Error::SUCCESS;
}

template<typename T>
Error Manager::publish(const char* name, const T& data) {
    TopicInfo* topic = findTopic(name);
    if (!topic || !topic->registered) {
        return Error::TOPIC_NOT_FOUND;
    }

    // 检查类型是否匹配（使用编译时类型ID）
    if (topic->type_id != type_traits::TypeID<T>::id() ||
        topic->data_size != sizeof(T)) {
        return Error::TYPE_MISMATCH;
    }

    // 复制数据
    std::memcpy(topic->data_ptr, &data, sizeof(T));
    return Error::SUCCESS;
}

template<typename T>
Error Manager::get(const char* name, T& data) {
    const TopicInfo* topic = findTopic(name);
    if (!topic || !topic->registered) {
        return Error::TOPIC_NOT_FOUND;
    }

    // 检查类型是否匹配
    if (topic->type_id != type_traits::TypeID<T>::id() ||
        topic->data_size != sizeof(T)) {
        return Error::TYPE_MISMATCH;
    }

    // 复制数据
    std::memcpy(&data, topic->data_ptr, sizeof(T));
    return Error::SUCCESS;
}

template<typename T>
T* Manager::getPtr(const char* name) {
    TopicInfo* topic = findTopic(name);
    if (!topic || !topic->registered) {
        return nullptr;
    }

    // 检查类型是否匹配
    if (topic->type_id != type_traits::TypeID<T>::id() ||
        topic->data_size != sizeof(T)) {
        return nullptr;
    }

    return static_cast<T*>(topic->data_ptr);
}

} // namespace dtm

// 便捷宏定义
#define DTM_DECLARE_TOPIC(type, name) \
extern dtm::TopicStorage<type> g_dtm_topic_##name

// 定义话题（在源文件中使用）
#define DTM_DEFINE_TOPIC(type, name) \
dtm::TopicStorage<type> g_dtm_topic_##name

// 注册话题（在源文件中使用，需要放在函数或初始化块中）
#define DTM_REGISTER_TOPIC(type, name) \
dtm::Manager::registerTopic<type>(#name, g_dtm_topic_##name)

// 获取话题引用
#define DTM_TOPIC_REF(name) \
g_dtm_topic_##name.get()

// 获取话题指针
#define DTM_TOPIC_PTR(name) \
g_dtm_topic_##name.ptr()

// 发布数据
#define DTM_PUBLISH(name, data) \
dtm::Manager::publish(#name, data)

// 获取数据
#define DTM_GET(name, data) \
dtm::Manager::get(#name, data)

// 获取数据指针
#define DTM_GET_PTR(type, name) \
dtm::Manager::getPtr<type>(#name)

// 便捷的全局注册宏
#define DTM_AUTO_REGISTER_TOPIC(type, name)\
namespace { \
struct DTM_Registrar_##name { \
DTM_Registrar_##name() { \
DTM_REGISTER_TOPIC(type, name); \
} \
}; \
static DTM_Registrar_##name dtm_registrar_##name; \
}
