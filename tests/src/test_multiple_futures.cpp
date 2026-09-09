#include <iostream>

#include <Lunaris/future.h>

#include "common.h"

using namespace Lunaris::Future;

int main() {
    std::printf("Testing multiple futures from a single promise...\n");
    srand(time(0));

    constexpr size_t amount = 20;
    const int base = rand() % 1000;
    std::atomic<size_t> sum{0}, expected_sum{0};

    Promise<Test> prom_mult;

    auto futures = prom_mult.get_multiple_future(amount);

    if (futures.size() != amount) {
        std::printf("Futures amount does not match expected amount!\n");
        return 1;
    }

    for(size_t p = 0; p < amount; ++p) {
        expected_sum += p * base;
        futures[p].then([idx = p,&sum](Test&& test) {
            sum += static_cast<size_t>((int)test) * idx;
        });
    }

    prom_mult.set(base);

    if (expected_sum != sum) {
        std::printf("Sum from futures does not match expected sum!\n");
        return 1;
    }

    if (Test::get_copies() != amount) {
        std::printf("Number of copies exceeded expected amount!\n");
        return 1;
    }
    if (Test::get_moves() != 1) {
        std::printf("Number of moves exceeded expected amount!\n");
        return 1;
    }

    std::printf("PASSED!\n");

    return 0;
}