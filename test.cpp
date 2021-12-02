
#include "Just/JustThreadPool.h"
#include "Just/JustConcurrentQueue.hpp"

#include <concurrentqueue/concurrentqueue.h>

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

#define COUNT (20000000)
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
real    0m40.486s
user    4m34.419s
sys     0m2.727s

real    0m31.319s
user    3m23.801s
sys     0m1.813s

real    0m40.144s
user    4m32.750s
sys     0m2.232s

real    0m31.404s
user    5m11.714s
sys     0m3.189s

real    0m36.197s
user    4m2.218s
sys     0m2.592s

real    0m37.537s
user    4m15.984s
sys     0m1.621s

real    0m37.334s
user    4m14.541s
sys     0m1.912s

real    0m43.380s
user    5m0.267s
sys     0m1.803s

real    0m42.796s
user    7m32.051s
sys     0m3.240

real    0m41.113s
user    7m11.081s
sys     0m2.721s
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
real    0m58.803s
user    5m52.643s
sys     0m0.485s

real    0m59.029s
user    5m52.246s
sys     0m0.695s

real    0m58.520s
user    5m50.931s
sys     0m0.775s

real    0m47.742s
user    5m47.107s
sys     0m1.309s

real    0m56.790s
user    5m46.230s
sys     0m0.726s

real    0m45.242s
user    5m31.653s
sys     0m0.521s

real    0m45.754s
user    5m32.650s
sys     0m0.451s

real    0m45.558s
user    5m31.938s
sys     0m0.591s

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
/*
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
*/

int main(int argc, char* argv[])
{
    // Just::ConcurrentQueue<int> cq;
    // test_queue01(cq);
    // test_queue02(cq);

    test_queue03<int>();

    return 0;
}