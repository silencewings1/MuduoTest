// #include "datatime/TimeStamp.h"
// #include <iostream>
// using namespace std::chrono_literals;

// int main()
// {
//     TimeStamp ts1(TimeStamp::Now());

//     auto ts2 = AddTime(ts1, 3h);

//     std::cout << ts1.ToString() << std::endl;
//     std::cout << ts2.ToString() << std::endl;

//     return 0;
// }

#include "EventLoop.h"
#include <chrono>
#include <iostream>
using namespace std::chrono_literals;

int main()
{
    EventLoop loop;

    int count = 0;

    auto print = [&](std::string msg) {
        std::cout << "msg: " << msg
                  << ", time: " << TimeStamp::Now().ToString() << std::endl;

        if (++count == 15)
        {
            loop.Quit();
        }
    };

    loop.RunAfter(1s, [&]() { print("once1s"); });
    loop.RunAfter(1500ms, [&]() { print("once1.5s"); });
    loop.RunAfter(2500ms, [&]() { print("once2.5s"); });
    loop.RunAfter(3500ms, [&]() { print("once3.5s"); });
    loop.RunEvery(2s, [&]() { print("every2s"); });
    loop.RunEvery(3s, [&]() { print("every3s"); });


    loop.Loop();

    return 0;
}