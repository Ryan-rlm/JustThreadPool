
#include <thread>
#include <vector>
//#include <shared_mutex>
#include <mutex>

#include "JustThreadPool.h"
#include "JustConcurrentQueue.hpp"
using namespace Just;


namespace
{
    const size_t KERNAL_COUNT = std::thread::hardware_concurrency();
    bool usefulThreadHint(size_t thread_hint)
    {
        return (thread_hint > 0) && (thread_hint <= KERNAL_COUNT * 2);
    }
}

struct ThreadPool::Data
{
    //std::counting_semaphore<PTRDIFF_MAX> task_sem;
    std::condition_variable_any task_sem;
    ConcurrentQueue<Task> task_queue; // 工作队列

    //std::shared_mutex pool_mutex;
    std::mutex pool_mutex;
    size_t thread_size;
    std::vector<std::thread> thread_vec;  // 线程池

    Status stat = Status::Inited;
    Order order = Order::None;
};

void ThreadPool::work_func()
{
    bool got = false;

    for (;;)
    {
        got = false;
        Task task;

        // wait
        got = d->task_queue.try_pop(task);
        if (got && task)
            task();

        if (d->order == Order::Stop)
        {
            break;
        }
        else if (d->order == Order::StopAndDone)
        {
            if (d->task_queue.empty())
                break;
        }
    }
}

void ThreadPool::task_enqueue(Task&& t)
{
    d->task_queue.try_push(std::move(t));
}

ThreadPool::ThreadPool()
    : d{ std::make_unique<Data>() }
{
    d->thread_size = KERNAL_COUNT;
    start(d->thread_size);
}

ThreadPool::ThreadPool(size_t thread_hint)
    : d{ std::make_unique<Data>() }
{
    d->thread_size = usefulThreadHint(thread_hint) ? thread_hint : KERNAL_COUNT;
    start(d->thread_size);
}

ThreadPool::~ThreadPool()
{
    stop(Order::StopAndDone);
}

size_t ThreadPool::thread_count() const
{
    return d->thread_size;
}

size_t ThreadPool::task_count() const
{
    return d->task_queue.size();
}

void ThreadPool::clear()
{
    d->task_queue.clear();
}

bool ThreadPool::start(size_t thread_hint/* = 0*/)
{
    //std::unique_lock<std::shared_mutex> locker(d->pool_mutex);
    std::lock_guard<std::mutex> locker(d->pool_mutex);

    if (d->stat != Status::Inited
        && d->stat != Status::Stoped)
        return false;

    d->stat = Status::Starting;
    d->order = Order::None;
    d->thread_size = usefulThreadHint(thread_hint) ? thread_hint : KERNAL_COUNT;
    d->thread_vec.clear();

    for (size_t i = 0; i < d->thread_size; i++)
    {
        d->thread_vec.emplace_back(&ThreadPool::work_func, this);
    }

    d->stat = Status::Running;

    return true;
}

ThreadPool::Status ThreadPool::status() const
{
    //std::shared_lock<std::shared_mutex> locker(d->pool_mutex);
    std::lock_guard<std::mutex> locker(d->pool_mutex);

    return d->stat;
}

void ThreadPool::stop(Order od)
{
    if (od == Order::None)
        return;

    //std::unique_lock<std::shared_mutex> locker(d->pool_mutex);
    std::lock_guard<std::mutex> locker(d->pool_mutex);

    d->stat = Status::Stopping;
    d->order = od;

    for (auto& it : d->thread_vec)
    {
        if (it.joinable())
            it.join();
    }

    d->thread_vec.clear();
    d->stat = Status::Stoped;
    d->order = Order::None;
}

ThreadPool& Just::commonThreadPool()
{
    static ThreadPool threadPool(7);

    return threadPool;
}
