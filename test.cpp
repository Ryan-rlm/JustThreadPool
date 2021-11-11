
#include "JustThreadPool.h"
#include "JustConcurrentQueue.hpp"

#include <memory>
#include <iostream>
#include <list>
#include <queue>
#include <atomic>
using namespace std;

void test()
{
    Just::ThreadPool pool(4);

    for(int i = 0; i < 8; ++i) {
        pool.run([i] {
            std::cout << "hello " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "world " << i << std::endl;
        });
    }
}

void test1()
{

    atomic_int tmp_int;
    atomic_init(&tmp_int, 10);

    cout << tmp_int.is_lock_free() << endl;

    int tmp = tmp_int.load();
    cout << tmp << endl;

    std::atomic_compare_exchange_strong(&tmp_int, &tmp, 30);

    cout << tmp_int.load() << endl;

    tmp_int.compare_exchange_strong(tmp, 13);
    cout << tmp_int.load() << endl;
}

struct NodeBase
{
    NodeBase* _next;
    NodeBase* _prev;
};

int main(int argc, char* argv[])
{
    atomic_int tmp_int;
    atomic_init(&tmp_int, 13);
    cout << tmp_int.load() << endl;
    atomic_init(&tmp_int, 19);
    cout << tmp_int.load() << endl;

    cout << sizeof(NodeBase) << endl;
    atomic<NodeBase> nb;

    Just::ConcurrentQueue<int> cq;
    cq.push(3);

    int tmp;
    cq.pop(tmp);
    cout << tmp << endl;

    return 0;
}