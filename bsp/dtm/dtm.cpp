/**
 * @file dtm.cpp
 * @brief 数据中转模块实现
 */

#include "dtm.h"
#include <cstring>

namespace dtm {

TopicInfo Manager::s_topics[DTM_MAX_TOPICS];
uint32_t Manager::s_topic_count = 0;

void Manager::init() {
    s_topic_count = 0;
    for (auto& topic : s_topics) {
        topic.registered = false;
        topic.name = nullptr;
        topic.data_ptr = nullptr;
        topic.data_size = 0;
        topic.type_id = 0;
    }
}

void Manager::clear() {
    s_topic_count = 0;
    // 注意：这里不清除TopicStorage的数据，只清除注册信息
}

bool Manager::exists(const char* name) {
    return findTopic(name) != nullptr;
}

// 常用类型的显式实例化（如果需要分离编译）
#ifdef DTM_SEPARATE_COMPILATION
template Error Manager::registerTopic<int>(const char*, TopicStorage<int>&);
template Error Manager::registerTopic<float>(const char*, TopicStorage<float>&);
template Error Manager::registerTopic<double>(const char*, TopicStorage<double>&);

template Error Manager::publish<int>(const char*, const int&);
template Error Manager::publish<float>(const char*, const float&);
template Error Manager::publish<double>(const char*, const double&);

template Error Manager::get<int>(const char*, int&);
template Error Manager::get<float>(const char*, float&);
template Error Manager::get<double>(const char*, double&);

template int* Manager::getPtr<int>(const char*);
template float* Manager::getPtr<float>(const char*);
template double* Manager::getPtr<double>(const char*);
#endif

} // namespace dtm