#ifndef TASK_INFO_HPP
#define TASK_INFO_HPP

#include <coroutine>
#include <cstdint>
struct task_info {
  std::coroutine_handle<> handle;
  int32_t res{};

  uint64_t as_user_data(){
    return  static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));
  }
};

inline constexpr uintptr_t task_info_ptr_mask = ~(alignof(task_info)-1);

static_assert(~task_info_ptr_mask==7);



#endif
