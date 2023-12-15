#ifndef TIME_CAST_HPP
#define TIME_CAST_HPP

#include <chrono>
namespace coroutine {

template <typename Ref, typename Period>
inline __kernel_timespec
to_kernel_timespec(std::chrono::duration<Ref, Period> duration) {
  using namespace std::chrono;
  auto sec = duration_cast<seconds>(duration);
  auto nsec = duration_cast<nanoseconds>(duration - sec);
  return __kernel_timespec{.tv_sec = sec.count(), .tv_nsec = nsec.count()};
} // namespace corotine
} // namespace coroutine
#endif
