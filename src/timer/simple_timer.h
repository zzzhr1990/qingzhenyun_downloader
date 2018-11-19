//
// Created by zzzhr on 2018/11/14.
//

#ifndef QINGZHENYUN_DOWNLOADER_SIMPLE_TIMER_H
#define QINGZHENYUN_DOWNLOADER_SIMPLE_TIMER_H

#include <atomic>
#include <mutex>

namespace qingzhen::timer{
    class simple_timer {
    public:
        simple_timer();
        simple_timer(const simple_timer& t);
        void start_timer(int interval, const std::function<void()> &task, const bool& immediately = false);
        void expire();
        ~simple_timer();
        template<typename callable, class... arguments>
        void sync_wait(int after, callable&& f, arguments&&... args);
        template<typename callable, class... arguments>
        void async_wait(int after, callable&& f, arguments&&... args);
    private:
        std::atomic<bool> _expired_{};
        std::atomic<bool> _try_to_expire_{};
        std::mutex _mutex_;
        std::condition_variable _expired_cond_;
    };
}

#endif //QINGZHENYUN_DOWNLOADER_SIMPLE_TIMER_H
