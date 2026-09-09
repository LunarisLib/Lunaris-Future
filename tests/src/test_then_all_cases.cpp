#include <iostream>
#include <thread>

#include <Lunaris/future.h>

using namespace Lunaris::Future;

int main() {
    Promise<int> prom;
    Future<int> nxt = prom.get_future()
        .then([](int val) { std::cout << "Thenn!: " << val << "\n"; return val * 2; })
        .then([](int val) { std::cout << "Then now return void, but got: " << val << "\n"; return; })
        .then([]() { std::cout << "Then void to void.\n"; return; })
        .then([]() { std::cout << "Then got void, but returning 10 again\n"; return 10; });

    std::thread thr([&] {
        nxt.wait();
        std::cout << "Got value, wait 1 sec...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Value set: " << nxt.get() << std::endl;
    });

    std::cout << "Wait...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    prom.set(64);
    thr.join();

    std::printf("PASSED!\n");
    
    return 0;
}
