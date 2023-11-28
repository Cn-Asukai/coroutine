//
// Created by Asukai on 2023/11/22.
//

#include "context.hpp"

namespace coroutine {
void co_spawn(task<> t) {
    this_thread.ctx->co_spawn(std::move(t));
  }
}

