#include "datatime/TimeStamp.h"
#include <iostream>
using namespace std::chrono_literals;

int main()
{
    TimeStamp ts1(TimeStamp::Now());

    auto ts2 = AddTime(ts1, 3h);

    std::cout << ts1.ToString() << std::endl;
    std::cout << ts2.ToString() << std::endl;

    return 0;
}