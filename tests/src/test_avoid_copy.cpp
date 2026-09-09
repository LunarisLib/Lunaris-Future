#include <iostream>

#include <Lunaris/future.h>

#include "common.h"

using namespace Lunaris::Future;

int main() {
    std::printf("Testing move through callbacks.\n");

    {
        Promise<Test> prom_test;
        prom_test.get_future().then([](Test&& test) { 
            std::cout << "Middle function: " << (int)test << std::endl;
            return test; 
        }).then([](Test&& test) {
            std::cout << "Last function: " << (int)test << std::endl;
        });

        prom_test.set(Test{rand() % 1000});

        if (Test::get_moves() != 1) {
            std::printf("Got more moves than necessary!\n");
            return 1;
        }
    }

    std::printf("Callbacks work!\n");
    std::printf("Testing move through get().\n");

    Test::get_moves() = 0;

    {
        Promise<Test> prom_test;
        Future<Test> fut_test = prom_test.get_future();

        prom_test.set(Test{rand() % 1000});
        Test gotten{fut_test.get()};

        if (Test::get_moves() != 2) {
            std::printf("Got more moves than necessary!\n");
            return 1;
        }
    }

    

    if (Test::get_copies() != 0) {
        std::printf("Library copied once! It shouldn've done that!\n");
        return 1;
    }

    std::printf("PASSED!\n");

    return 0;
}