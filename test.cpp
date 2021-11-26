
#include "Just/JustThreadPool.h"
#include "Just/JustConcurrentQueue.hpp"

#include <concurrentqueue/concurrentqueue.h>
#include <boost/lockfree/queue.hpp>

#include <cstddef>
#include <memory>
#include <list>
#include <queue>
#include <atomic>
#include <sys/types.h>
#include <typeinfo>
#include <vector>
#include <thread>
#include <iostream>
using namespace std;

#define COUNT (10000000)
const size_t PUSH_THREADS = (10);
const size_t POP_THREADS = (7);

// push
template<typename T, const size_t Count = COUNT>
void test_queue01(Just::ConcurrentQueue<T>& cq)
{
    vector<thread> push_threads;

    cout << "push" << endl;
    cout << "cq empyt: " << cq.empty() << endl;
    cout << "cq size: " << cq.size() << endl;
    for (size_t i = 0; i < 10; i++)
    {
        push_threads.emplace_back([i, &cq](){
            for (size_t j = 0; j < Count; j++)
            {
                cq.push(T());
            }
        });
    }

    for (auto& it : push_threads)
    {
        it.join();
    }

    cout << "cq empyt: " << cq.empty() << endl;
    cout << "cq size: " << cq.size() << endl;
    cout << "pushend" << endl;
}

// pop
template<typename T, const size_t Count = COUNT>
void test_queue02(Just::ConcurrentQueue<T>& cq)
{
    vector<thread> pop_threads;
    atomic_uint32_t pop_num(0);
    cout << "pop" << endl;
    cout << "cq empyt: " << cq.empty() << endl;
    cout << "cq size: " << cq.size() << endl;

    for (size_t i = 0; i < 10; i++)
    {
        pop_threads.emplace_back([i, &cq, &pop_num](){
            T tmp;
            for (size_t j = 0; j < Count; j++)
            {
                if (cq.pop(tmp))
                    ++pop_num;
            }
        });
    }

    for (auto& it : pop_threads)
    {
        it.join();
    }

    cout << "cq empyt: " << cq.empty() << endl;
    cout << "cq size: " << cq.size() << endl;
    cout << "popend " << pop_num << endl;
}

/*
real    0m17.903s
user    3m4.478s
sys     0m0.890s

real    0m16.795s
user    2m52.332s
sys     0m1.090s

real    0m16.582s
user    2m43.925s
sys     0m1.312s

real    0m17.330s
user    2m58.430s
sys     0m0.859s

*/
// push and pop
template<typename T, const size_t Count = COUNT>
void test_queue03()
{
    atomic_uint32_t pop_num;
    atomic_init(&pop_num, 0U);
    vector<thread> push_threads;
    vector<thread> pop_threads;

    Just::ConcurrentQueue<int> cq;

    for (size_t i = 0; i < PUSH_THREADS; i++)
    {
        push_threads.emplace_back([i, &cq](){
            cout << "push start" << endl;
            for (size_t j = 0; j < Count; j++)
            {
                cq.push(j * i);
            }
            cout << "push end" << endl;
            });
    }

    for (size_t i = 0; i < POP_THREADS; i++)
    {
        pop_threads.emplace_back([i, &cq, &pop_num](){
            cout << "pop start" << endl;
            int tmp;
                for (size_t i = 0; i < Count * 10; ++i)
                {
                    if (cq.pop(tmp))
                        ++pop_num;
                }
                cout << "pop end" << endl;
            });
    }

    for (auto& it : push_threads)
    {
        it.join();
    }

    for (auto& it : pop_threads)
    {
        it.join();
    }

    cout << "pop num: " << pop_num << endl;
    cout << "cq empyt: " << cq.empty() << endl;
    cout << "cq size: " << cq.size() << endl;
}

/*

real    0m22.845s
user    2m46.740s
sys     0m0.321s

real    0m22.743s
user    2m46.700s
sys     0m0.451s
*/
template<typename T, const size_t Count = COUNT>
void test_queue04()
{
    atomic_uint32_t pop_num;
    atomic_init(&pop_num, 0U);
    vector<thread> push_threads;
    vector<thread> pop_threads;

    moodycamel::ConcurrentQueue<T> cq;

    for (size_t i = 0; i < PUSH_THREADS; i++)
    {
        push_threads.emplace_back([i, &cq](){
            cout << "push start" << endl;
            for (size_t j = 0; j < Count; j++)
            {
                cq.enqueue(j * i);
            }
            cout << "push end" << endl;
            });
    }

    for (size_t i = 0; i < POP_THREADS; i++)
    {
        pop_threads.emplace_back([i, &cq, &pop_num](){
            cout << "pop start" << endl;
            int tmp;
                for (size_t i = 0; i < Count * 10; ++i)
                {
                    if (cq.try_dequeue(tmp))
                        ++pop_num;
                }
                cout << "pop end" << endl;
            });
    }

    for (auto& it : push_threads)
    {
        it.join();
    }

    for (auto& it : pop_threads)
    {
        it.join();
    }

    cout << "pop num: " << pop_num << endl;
    // cout << "cq empyt: " << cq.size_approx() << endl;
    cout << "cq size: " << cq.size_approx() << endl;
}

/*
10000000
*/

template<typename T, const size_t Count = COUNT>
void test_queue05()
{
    atomic_uint32_t pop_num;
    atomic_init(&pop_num, 0U);
    vector<thread> push_threads;
    vector<thread> pop_threads;

    //moodycamel::ConcurrentQueue<T> cq;
    boost::lockfree::queue<T> cq(1000);
    cq.reserve(Count);

    for (size_t i = 0; i < PUSH_THREADS; i++)
    {
        push_threads.emplace_back([i, &cq](){
            cout << "push start" << endl;
            for (size_t j = 0; j < Count; j++)
            {
                cq.push(i * j);
            }
            cout << "push end" << endl;
            });
    }

    for (size_t i = 0; i < POP_THREADS; i++)
    {
        pop_threads.emplace_back([i, &cq, &pop_num](){
            cout << "pop start" << endl;
            int tmp;
                for (size_t i = 0; i < Count * 10; ++i)
                {
                    if (cq.pop(tmp))
                        ++pop_num;
                }
                cout << "pop end" << endl;
            });
    }

    for (auto& it : push_threads)
    {
        it.join();
    }

    for (auto& it : pop_threads)
    {
        it.join();
    }

    cout << "pop num: " << pop_num << endl;
    // cout << "cq empyt: " << cq.size_approx() << endl;
    // cout << "cq size: " << cq.fixed_sized() << endl;
}

int main(int argc, char* argv[])
{
    // Just::ConcurrentQueue<int> cq;
    // test_queue01(cq);
    // test_queue02(cq);

    test_queue03<int>();

    return 0;
}