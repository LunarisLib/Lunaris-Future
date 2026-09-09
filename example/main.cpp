#include <iostream>

#include <Lunaris/future.h>

class Test {
    int m_val{};
public:
    Test(int val) : m_val(val) {}
    Test(const Test& t) : m_val(t.m_val) { std::printf("copy constr\n"); }
    Test(Test&& t) : m_val(t.m_val)  { std::printf("move constr\n"); }
    void operator=(const Test& t) { m_val = t.m_val; std::printf("copy op\n"); }
    void operator=(Test&& t) { m_val = t.m_val; std::printf("move op\n"); }

    operator int() const { return m_val; }
};

using namespace Lunaris::Future;

int main() {
    {
        Promise<int> prom;
        Future<int> fut = prom.get_future();
        auto nxt = fut.then([](int v) { return v * 3.14f; } ).then([](float f) { return std::to_string(f);} );

        prom.set(10);
        std::cout << nxt.get() << std::endl;
    }

    {
        Promise<Test> prom_test;
        Future<Test> fut_test = prom_test.get_future();
        Future<Test> next_test = fut_test.then([](Test&& test) { 
            std::cout << "MIDDLE: " << (int)test << std::endl;
            return test; 
        });

        next_test.then([](Test&& test) {
            std::cout << "TEST: " << (int)test << std::endl;
        });

        prom_test.set(Test{5});
        prom_test.set(Test{15});
        prom_test.set(Test{25});
    }

    {
        Promise<Test> prom_mult;
        auto futures = prom_mult.get_multiple_future(10);

        for(size_t p = 0; p < 10; ++p) {
            futures[p].then([idx = p](Test&& test) {
                std::cout << "MULT #" << idx << ": " << (int)test << std::endl;
            });
        }

        prom_mult.set(rand() % 1000);
    }




    std::printf("End\n");

    return 0;
}


//{
//        pipe<Test> test;
//        //test.make_callback([](Test&& t) {
//        //    std::printf("Called back %d\n", (int)t);
//        //});
//
//        std::printf("Set\n");
//        test = Test{50};
//
//        std::printf("Get\n");
//        const Test got = test.get();
//}

/*
int main() {
    std::printf("Testing simple promise future get.\n");

    Promise<int> prom_random_int;
    Future<int> getting_random_int = prom_random_int.get_future();

    if (getting_random_int.wait(std::chrono::milliseconds(10))) {
        std::printf("Future wasn't supposed to be set yet.\n");
        return 1;
    }

    const int random_num = rand() % 1000;
    prom_random_int.set_value(random_num);

    if (!getting_random_int.wait(std::chrono::milliseconds(0))) {
        std::printf("Future was supposed to be set already.\n");
        return 1;
    }

    if (getting_random_int.get() != random_num) {
        std::printf("Future got wrong result?\n");
        return 1;        
    }
    if (getting_random_int.get() != random_num) {
        std::printf("Future cannot get twice?\n");
        return 1;        
    }
    if (getting_random_int.get_take() != random_num) {
        std::printf("Future cannot get_take?\n");
        return 1;        
    }
    if (getting_random_int.wait(std::chrono::milliseconds(10))) {
        std::printf("Future wasn't supposed to be set after get_take.\n");
        return 1;
    }

    std::printf("PASSED!\n");
    
    return 0;
}*/