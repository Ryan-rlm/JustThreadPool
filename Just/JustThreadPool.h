
#pragma once
#ifndef __JUSTTHREADPOOL_H__
#define __JUSTTHREADPOOL_H__

#include <future>
#include <memory>
#include <functional>

#include "JustConfig.h"

JUST_NSP_START
using Task = std::function<void()>;

class ThreadPool final
{
public:
    enum class Status
    {
        None,
        Inited,    // 刚初始化
        Starting,  // 开始创建线程池
        Running,   // 正在执行
        Stopping,  // 正在停止
        Stoped,    // 已停止
    };

    enum class Order
    {
        None,
        Stop,
        StopAndDone,
    };

private:
    struct Data;
    std::unique_ptr<Data> d;

    void work_func();
    void task_enqueue(Task&& t);
    void task_enqueue(Task& t);

public:
    ThreadPool();
    ThreadPool(size_t thread_hint);
    ~ThreadPool();

    size_t thread_count() const;
    size_t task_count() const;

    template<typename Func, typename... Args>
    std::future<std::result_of_t<std::decay_t<Func>(std::decay_t<Args>...)>>
        run(Func&& func, Args&&... args)
    {
        using ret_t = typename std::result_of_t<std::decay_t<Func>(std::decay_t<Args>...)>;
        auto pkg_task = std::make_shared<std::packaged_task<ret_t(std::decay_t<Args>...)>>(std::forward<Func>(func), std::forward<Args>(args)...);

        task_enqueue([pkg_task]() { (*pkg_task)(); });

        return pkg_task->get_future();
    }

    void clear();

    bool start(size_t thread_hint = 0);
    Status status() const;
    void stop(Order od = Order::StopAndDone);
};

ThreadPool& commonThreadPool();

template<typename Func, typename... Args>
std::future<std::result_of_t<std::decay_t<Func>(std::decay_t<Args>...)>>
    async(ThreadPool& threadPool, Func&& func, Args&&... args)
{
    return threadPool.run(std::forward<Func>(func), std::forward<Args>(args)...);
}

template<typename Func, typename... Args>
std::future<std::result_of_t<std::decay_t<Func>(std::decay_t<Args>...)>>
    async(Func&& func, Args&&... args)
{
    return commonThreadPool().run(std::forward<Func>(func), std::forward<Args>(args)...);
}

JUST_NSP_END

#endif // __JUSTTHREADPOOL_H__
