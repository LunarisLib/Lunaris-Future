#pragma once

#include <cstdio>

class Test {
    int m_val{};
    static size_t g_copies;
    static size_t g_moves;
public:
    Test(int val) : m_val(val) {}
    Test(const Test& t) : m_val(t.m_val) {
        std::printf("copy constr\n");
        ++g_copies;
    }
    Test(Test&& t) : m_val(t.m_val) {
        std::printf("move constr\n");
        ++g_moves;
    }
    void operator=(const Test& t) {
        m_val = t.m_val; std::printf("copy op\n");
        ++g_copies;
    }
    void operator=(Test&& t) {
        m_val = t.m_val; std::printf("move op\n");
        ++g_moves;
    }

    static size_t& get_copies() { return g_copies; }
    static size_t& get_moves() { return g_moves; }

    operator int() const { return m_val; }
};

inline size_t Test::g_copies = 0;
inline size_t Test::g_moves = 0;