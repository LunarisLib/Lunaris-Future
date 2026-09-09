#include <iostream>

#include <Lunaris/future.h>

using namespace Lunaris::Future;

int main() {
    std::printf("Testing promise as tunnel.\n");

    Promise<int> prom_random_int;
    auto fut = prom_random_int.get_future()
        .then([](int res){ return res * 2; });

    prom_random_int.set(10);

    if (fut.get() != 20) {
        std::printf("Future did not work as expected.\n");
        return 1;
    }

    std::printf("Re-set of promise...\n");

    prom_random_int.set(20);

    if (fut.get() != 40) {
        std::printf("Future did not work as expected.\n");
        return 1;
    }

    std::printf("PASSED!\n");
    
    return 0;
}