
#include "JustConcurrentQueue.hpp"

#include <memory>
#include <iostream>
#include <list>
#include <queue>
#include <atomic>
#include <vector>
#include <thread>
using namespace std;


void test1()
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

int main(int argc, char* argv[])
{
    // shared_ptr<int> int_ptr = make_shared<int>(30);
    // auto ptr2 = atomic_load(&int_ptr);
    // *ptr2 = 40;
    // // test1();

    // cout << *int_ptr << endl;

    test1();


    return 0;
}