
#include "Just/JustThreadPool.h"
#include "Just/JustConcurrentQueue.hpp"

#include <cstddef>
#include <memory>
#include <list>
#include <queue>
#include <atomic>
#include <vector>
#include <thread>
#include <iostream>
using namespace std;

#define COUNT (2222222)

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

// push and pop
void test_queue03()
{
    const int test_num = 2222222; //1000000
    atomic_uint32_t pop_num;
    atomic_init(&pop_num, 0U);
    vector<thread> push_threads;
    vector<thread> pop_threads;

    Just::ConcurrentQueue<int> cq;

    for (size_t i = 0; i < 10; i++)
    {
        push_threads.emplace_back([i, &cq](){
            for (size_t j = 0; j < test_num; j++)
            {
                cq.push(j * i);
            }
        });
    }

    for (size_t i = 0; i < 5; i++)
    {
        pop_threads.emplace_back([i, &cq, &pop_num](){
            int tmp;
            for (size_t i = 0; i < test_num * 10; ++i)
            {
                if (cq.pop(tmp))
                    ++pop_num;
            }
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

void test_pool()
{
    Just::commonThreadPool().run([](){
        printf("Hello world!\n");
    });
}

int main(int argc, char* argv[])
{
    Just::ConcurrentQueue<int> cq;
    test_queue01(cq);
    test_queue02(cq);

    return 0;
}