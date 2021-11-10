
#include "JustThreadPool.h"

#include <iostream>
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

int main(int argc, char* argv[])
{
    test();

    return 0;
}