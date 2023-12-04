#ifndef USER_DATA_HPP
#define USER_DATA_HPP
#include <cstdint>

namespace coroutine {

enum class reserved_user_data : uint64_t { co_spawn_event, none };

enum class user_data_type {
  task_info_ptr

};

} // namespace coroutine

#endif
