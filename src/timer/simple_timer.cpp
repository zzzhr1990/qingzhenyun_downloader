//
// Created by zzzhr on 2018/11/14.
//

#include "simple_timer.h"
#include <thread>
#include <iostream>
qingzhen::timer::simple_timer::simple_timer(const qingzhen::timer::simple_timer &t) {
    _expired_ = t._expired_.load();
    _try_to_expire_ = t._try_to_expire_.load();
}

qingzhen::timer::simple_timer::~simple_timer() {
    expire();
}

qingzhen::timer::simple_timer::simple_timer() {
    _expired_ = true;
    _try_to_expire_ = false;
}

void
qingzhen::timer::simple_timer::start_timer(int interval, const std::function<void()> &task, const bool &immediately) {
    if (_expired_ == false){
        //          std::cout << "timer is currently running, please expire it first..." << std::endl;
        return;
    }
    if(immediately){
        task();
    }
    _expired_ = false;
    std::thread([this, interval, task](){
        int tick_interval = 100;
        int tick_time = interval / tick_interval;
        if(tick_time < 1){
            tick_time = 1;
        }
        int real_tick_time = 0;
        while (!_try_to_expire_){
            // while (!_try_to_expire_ && tick_time )

            auto start = std::chrono::system_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(tick_interval));
            auto end = std::chrono::system_clock::now();
            auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            if(dur > tick_interval + 500){
                std::cout << "Timer tick_interval slow: "  << dur << " milliseconds" << std::endl;
            }
            //dur.
            if(!_try_to_expire_ && real_tick_time == tick_time){
                task();
                real_tick_time = 0;
            }
            real_tick_time ++;
        }
        //          std::cout << "stop task..." << std::endl;
        {
            std::lock_guard<std::mutex> locker(_mutex_);
            _expired_ = true;
            _expired_cond_.notify_one();
        }
    }).detach();
}

void qingzhen::timer::simple_timer::expire() {
    if (_expired_){
        return;
    }

    if (_try_to_expire_){
        return;
    }
    _try_to_expire_ = true;
    {
        std::unique_lock<std::mutex> locker(_mutex_);
        _expired_cond_.wait(locker, [this]{return _expired_ == true; });
        if (_expired_ == true){
            _try_to_expire_ = false;
        }
    }
}

template<typename callable, class... arguments>
void qingzhen::timer::simple_timer::sync_wait(int after, callable &&f, arguments &&... args) {
    std::function<typename std::result_of<callable(arguments...)>::type()> task
            (std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));
    std::this_thread::sleep_for(std::chrono::milliseconds(after));
    task();
}

template<typename callable, class... arguments>
void qingzhen::timer::simple_timer::async_wait(int after, callable &&f, arguments &&... args) {
    std::function<typename std::result_of<callable(arguments...)>::type()> task
            (std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

    std::thread([after, task](){
        std::this_thread::sleep_for(std::chrono::milliseconds(after));
        task();
    }).detach();
}
