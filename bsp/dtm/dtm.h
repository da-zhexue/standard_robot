#pragma once

#include <cstring>
#include <cstdint>
#include "ulog/ulog.h"
// 配置参数
#ifndef DTM_MAX_TOPICS
#define DTM_MAX_TOPICS 32
#endif

#ifndef DTM_MAX_NAME_LENGTH
#define DTM_MAX_NAME_LENGTH 64
#endif

namespace dtm {

enum class DTM_Error {
    SUCCESS = 0,
    TOPIC_NOT_FOUND,
    TOPIC_ALREADY_EXISTS,
    TYPE_MISMATCH,
    BUFFER_FULL,
    INVALID_PARAM
};

struct TopicInfo {
    const char* name;       // 话题名称
    void* data_ptr;         // 数据指针
    size_t data_size;       // 数据大小
    bool registered;        // 是否已注册

    constexpr TopicInfo()
        : name(nullptr), data_ptr(nullptr), data_size(0), registered(false) {}

    constexpr TopicInfo(const char* n, void* ptr, const size_t sz)
        : name(n), data_ptr(ptr), data_size(sz), registered(true) {}
};

template<typename T>
class TopicStorage {
private:
    T data_;

public:
    T& get() { return data_; }
    const T& get() const { return data_; }
    T* ptr() { return &data_; }
    const T* ptr() const { return &data_; }
    static constexpr size_t size() { return sizeof(T); }
};

class Manager {
private:

     static TopicInfo s_topics[DTM_MAX_TOPICS];
     static uint32_t s_topic_count;

     static TopicInfo* findTopic(const char* name);

public:
    // 禁止实例化
    Manager() = delete;
    ~Manager() = delete;
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    static void init();

    template<typename T>
    static DTM_Error registerTopic(const char* name, TopicStorage<T>& storage);

    template<typename T>
    static DTM_Error publish(const char* name, const T& data);

    template<typename T>
    static DTM_Error get(const char* name, T& data);

    template<typename T>
    static T* getPtr(const char* name);

    static bool exists(const char* name);
    static uint32_t count() { return s_topic_count; }
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
DTM_Error Manager::registerTopic(const char* name, TopicStorage<T>& storage) {
    // 检查缓冲区是否已满
    if (s_topic_count >= DTM_MAX_TOPICS) {
        return DTM_Error::BUFFER_FULL;
    }

    // 检查是否已存在同名话题
    if (findTopic(name) != nullptr) {
        return DTM_Error::TOPIC_ALREADY_EXISTS;
    }

    // 注册话题
    s_topics[s_topic_count] = TopicInfo(
        name,
        storage.ptr(),
        storage.size()
    );

    ++s_topic_count;
    return DTM_Error::SUCCESS;
}

template<typename T>
DTM_Error Manager::publish(const char* name, const T& data) {
    TopicInfo* topic = findTopic(name);
    if (!topic || !topic->registered) {
        return DTM_Error::TOPIC_NOT_FOUND;
    }

    if (topic->data_size != sizeof(T)) {
        return DTM_Error::TYPE_MISMATCH;
    }

    std::memcpy(topic->data_ptr, &data, sizeof(T));
    return DTM_Error::SUCCESS;
}

template<typename T>
DTM_Error Manager::get(const char* name, T& data) {
    const TopicInfo* topic = findTopic(name);
    if (!topic || !topic->registered) {
        return DTM_Error::TOPIC_NOT_FOUND;
    }

    if (topic->data_size != sizeof(T)) {
        return DTM_Error::TYPE_MISMATCH;
    }

    std::memcpy(&data, topic->data_ptr, sizeof(T));
    return DTM_Error::SUCCESS;
}

template<typename T>
T* Manager::getPtr(const char* name) {
    TopicInfo* topic = findTopic(name);
    if (!topic || !topic->registered) {
        return nullptr;
    }

    if (topic->data_size != sizeof(T)) {
        return nullptr;
    }

    return static_cast<T*>(topic->data_ptr);
}

} // namespace dtm

// 便捷宏定义
// 引入外部话题 在文件中使用该宏定义后可以使用下面两条宏定义直接获得话题数据而不经过Manager的查找
#define DTM_DECLARE_TOPIC(type, name) \
extern dtm::TopicStorage<type> g_dtm_topic_##name

// 获取话题引用
#define DTM_TOPIC_REF(name) \
g_dtm_topic_##name.get()

// 获取话题指针
#define DTM_TOPIC_PTR(name) \
g_dtm_topic_##name.ptr()

// 定义话题
#define DTM_DEFINE_TOPIC(type, name) \
dtm::TopicStorage<type> g_dtm_topic_##name

// 注册话题
#define DTM_REGISTER_TOPIC(type, name) \
dtm::Manager::registerTopic<type>(#name, g_dtm_topic_##name)

// 发布数据
#define DTM_PUBLISH(name, data) \
dtm::Manager::publish(#name, data)

// 获取数据
#define DTM_GET(name, data) \
dtm::Manager::get(#name, data)

// 获取数据指针
#define DTM_GET_PTR(type, name) \
dtm::Manager::getPtr<type>(#name)
