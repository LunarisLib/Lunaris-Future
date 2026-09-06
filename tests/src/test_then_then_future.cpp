#include <iostream>

#include <Lunaris/future.h>

using namespace Lunaris::Future;

int main() {
    std::printf("Testing promise then loop future.\n");

    Promise<int> prom_random_int;
    prom_random_int.get_future()
        .then([](int res){ return res * 2; });

    std::printf("PASSED!\n");
    
    return 0;
}