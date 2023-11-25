//
// Created by Asukai on 2023/11/22.
//

#ifndef THREAD_META_HPP
#define THREAD_META_HPP


namespace coroutine {

class context;

class thread_meta {
public:
    context* ctx=nullptr;
};

extern thread_local thread_meta this_thread;

}




#endif //THREAD_META_HPP
