
#include "JustThreadPool.h"
using namespace Just;


namespace
{
    const size_t KERNAL_COUNT = std::thread::hardware_concurrency();
    bool usefulThreadHint(size_t thread_hint)
    {
        return (thread_hint > 0) && (thread_hint <= KERNAL_COUNT * 2);
    }
}


void ThreadPool::work_func()
{
    bool got = false;
    
    for (;;)
    {
        got = false;
        Task task;

        got = task_queue.wait_dequeue_timed(task, std::chrono::seconds(1));
        if (got)
            task();

        if (order == Order::Stop)
        {
            break;
        }
        else if (order == Order::StopAndDone)
        {
            if (task_queue.size_approx() == 0)
                break;
        }
    }
}

ThreadPool::ThreadPool()
    : task_queue()
    , pool_mutex()
    , thread_size(KERNAL_COUNT)
    , thread_vec()
    , stat(Status::Inited)
    , order(Order::None)
{
    start(thread_size);
}

ThreadPool::ThreadPool(size_t thread_hint)
    : task_queue()
    , pool_mutex()
    , thread_size(usefulThreadHint(thread_hint) ? thread_hint : KERNAL_COUNT)
    , thread_vec()
    , stat(Status::Inited)
    , order(Order::None)
{
    start(thread_size);
}

ThreadPool::~ThreadPool()
{
    stop(Order::StopAndDone);
}

size_t ThreadPool::thread_count() const
{
    return thread_size;
}

size_t ThreadPool::task_count_approx() const
{
    return task_queue.size_approx();
}

void ThreadPool::clear()
{
    std::lock_guard locker(pool_mutex);
    
}

bool ThreadPool::start(size_t thread_hint/* = 0*/)
{
    std::lock_guard locker(pool_mutex);
    if (stat != Status::Inited
        && stat != Status::Stoped)
        return false;

    stat = Status::Starting;
    order = Order::None;
    thread_size = usefulThreadHint(thread_hint) ? thread_hint : KERNAL_COUNT;
    thread_vec.clear();

    for (size_t i = 0; i < thread_size; i++)
    {
        thread_vec.emplace_back(&ThreadPool::work_func, this);
    }

    stat = Status::Running;

    return true;
}

ThreadPool::Status ThreadPool::status()
{
    std::lock_guard locker(pool_mutex);

    return stat;
}

void ThreadPool::stop(Order od)
{
    if (od == Order::None)
        return;
    
    std::lock_guard locker(pool_mutex);
    stat = Status::Stopping;
    order = od;

    for (auto& it : thread_vec)
    {
        if (it.joinable())
            it.join();
    }
    
    thread_vec.clear();
    stat = Status::Stoped;
    order = Order::None;
}

ThreadPool* Just::commonThreadPool()
{
    static ThreadPool threadPool;

    return &threadPool;
}
