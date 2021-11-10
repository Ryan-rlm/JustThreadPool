# JustThreadPool

Just a thread pool

## Usage

```cpp
#include <iostream>
#include "JustTHreadPool.h"

int main(int argc, char* argv[])
{
    Just::ThreadPool tpool(4);

    tpool.run([](){
        cout << "Hello world!" << endl;
    });

    Just::async(tpool, [](){
        cout << "Hello world!" << endl;
    });

    return 0;
}

```
