#include <iostream>

#include <Lunaris/future.h>

using namespace Lunaris::Future;

int main() {
    std::printf("Testing simple promise future get.\n");

    Promise<int> prom_random_int;
    Future<int> getting_random_int = prom_random_int.get_future();

    if (getting_random_int.wait(std::chrono::milliseconds(10)) != e_wait_status::VALUE_UNSET_TIMEOUT) {
        std::printf("Future wasn't supposed to be set yet.\n");
        return 1;
    }

    const int random_num = rand() % 1000;
    prom_random_int.set(random_num);


    if (getting_random_int.wait(std::chrono::milliseconds(0)) != e_wait_status::VALUE_SET) {
        std::printf("Future was supposed to have value set.\n");
        return 1;
    }
    if (getting_random_int.get() != random_num) {
        std::printf("Future got wrong result?\n");
        return 1;        
    }
    if (getting_random_int.wait(std::chrono::milliseconds(0)) != e_wait_status::VALUE_REDIRECTED) {
        std::printf("Future was supposed to have value set.\n");
        return 1;
    }

    std::printf("PASSED!\n");
    
    return 0;
}