#pragma once

#include <cstddef>
#include <cstdint>

namespace dtm::type_traits
{

    // 编译时类型ID生成器
    template<typename T>
    struct TypeID {
        static constexpr uint64_t id() {
            // 使用函数局部静态变量的地址作为类型唯一标识
            static constexpr char dummy = 0;
            return reinterpret_cast<uint64_t>(&dummy);
        }

        static constexpr size_t size() {
            return sizeof(T);
        }

        static constexpr bool is_same(const uint64_t other_id) {
            return id() == other_id;
        }
    };

    // 类型ID比较器
    template<typename T1, typename T2>
    constexpr bool same_type() {
        return TypeID<T1>::id() == TypeID<T2>::id();
    }

}
