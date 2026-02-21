/**
 * @file dtm.cpp
 * @brief 数据中转模块实现
 * @TODO: 增加线程锁
 */

#include "dtm/dtm.h"
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
    }
}

void Manager::clear() {
    s_topic_count = 0;
}

bool Manager::exists(const char* name) {
    return findTopic(name) != nullptr;
}

} // namespace dtm