#pragma once
#include <iostream>
#include <random>

#include "virtual_template_functions.hpp"

namespace VTF_tests {

using VTF::enable_virtual_template_functions;
namespace meta = std::meta;

namespace test1 {
struct A;
struct B;
struct C;

struct base : enable_virtual_template_functions<A, B, C> {
    template <typename T>
    base(T* ptr) : enable_virtual_template_functions(ptr) {}

    template <typename T>
    void f() {
        constexpr auto default_func = [](base*) {};
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, default_func);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B, C);
    }
    virtual ~base() {}
};

struct A : base {
    static inline int con = 0;
    static inline int calls = 0;
    A() : base(this) { con++; }
    ~A() { con--; }
    template <typename T>
    void f() {
        calls++;
    }
};

struct B : base {
    static inline int con = 0;
    static inline int calls = 0;
    B() : base(this) { con++; }
    ~B() { con--; }
    template <typename T>
    void f() {
        calls++;
    }
};

struct C : base {
    static inline int con = 0;
    static inline int calls = 0;
    C() : base(this) { con++; }
    ~C() { con--; }
    template <typename T>
    void f() {
        calls++;
    }
};

bool run() {
    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(1, 3);
    bool ret = true;
    using ptrT = std::unique_ptr<base>;
    std::vector<ptrT> ptrs;
    for (int i = 0; i < 1000; i++) {
        int rand = uniform_dist(e1);
        if (rand == 1) {
            ptrs.emplace_back(std::make_unique<A>());
        } else if (rand == 2) {
            ptrs.emplace_back(std::make_unique<B>());
        } else {
            ptrs.emplace_back(std::make_unique<C>());
        }
    }
    for (auto& ptr : ptrs) {
        ptr->f<int>();
    }
    if (A::con != A::calls || B::con != B::calls || C::con != C::calls) {
        std::cout << "test1, error: virtual tempaltes functions did not get "
                     "called properly"
                  << std::endl;
        ret = false;
    }
    ptrs.clear();
    if (A::con != 0 || B::con != 0 || C::con != 0) {
        std::cout << "test1, error: derived classes did not destruct properly"
                  << std::endl;
        ret = false;
    }
    return ret;
}
}  // namespace test1

namespace test2 {
struct A;
struct B;
struct base : enable_virtual_template_functions<A, B> {
    template <typename T>
    base(T ptr) : enable_virtual_template_functions(ptr) {}

    template <typename T>
    char f() {
        constexpr auto base_f = [](base*) { return 'z'; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }

    template <auto i>
    int f() {
        constexpr auto base_f = [](base*) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(meta::reflect_constant(i), base_f);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }
};

struct A : base {
    A() : base(this) {}
    template <typename T>
    char f() {
        return 'a';
    }
    template <auto i>
    int f() {
        return 12;
    }
};

struct B : base {
    B() : base(this) {}

    template <auto i>
    int f() {
        return 13;
    }
    template <typename T>
    char f() {
        return 'b';
    }
};

bool run() {
    std::unique_ptr<base> a(std::make_unique<A>());
    std::unique_ptr<base> b(std::make_unique<B>());
    bool ret = true;
    if (a->f<int>() != 'a') {
        std::cout << "test2, error: a->f<int>() = " << a->f<int>()
                  << ", not \'a\'" << std::endl;
        ret = false;
    }
    if (a->f<1>() != 12) {
        std::cout << "test2, error: a->f<1>() = " << a->f<1>() << ", not 12"
                  << std::endl;
        ret = false;
    }
    if (b->f<int>() != 'b') {
        std::cout << "test2, error: b->f<int>() = " << b->f<int>()
                  << ", not \'b\'" << std::endl;
        ret = false;
    }
    if (b->f<1>() != 13) {
        std::cout << "test2, error: b->f<1>() = " << b->f<1>() << ", not 13"
                  << std::endl;
        ret = false;
    }
    return ret;
}

}  // namespace test2

namespace test3 {
struct A;
struct B;
struct base : enable_virtual_template_functions<A, B> {
    template <typename T>
    base(T ptr) : enable_virtual_template_functions(ptr) {}

    template <typename T>
    int f(int a) {
        constexpr auto base_f = [](base*, int) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }
    template <typename T>
    char f(char a) {
        constexpr auto base_f = [](base*, char) { return 'z'; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }
};

struct A : base {
    A() : base(this) {}

    template <typename T>
    int f(int a) {
        return 12;
    }
    template <typename T>
    char f(char a) {
        return 'a';
    }
};
struct B : base {
    B() : base(this) {}

    template <typename T>
    int f(int a) {
        return 13;
    }
    template <typename T>
    char f(char a) {
        return 'b';
    }
};

bool run() {
    bool ret = true;
    std::unique_ptr<base> a(std::make_unique<A>());
    std::unique_ptr<base> b(std::make_unique<B>());
    if (a->f<int>(1) != 12) {
        std::cout << "test3, error: a->f<int>(int) = " << a->f<int>(1)
                  << ", not 12" << std::endl;
        ret = false;
    }
    if (a->f<int>('a') != 'a') {
        std::cout << "test3, error: a->f<int>(char) = " << a->f<int>('a')
                  << ", not \'a\'" << std::endl;
        ret = false;
    }
    if (b->f<int>(1) != 13) {
        std::cout << "test3, error: b->f<int>(int) = " << a->f<int>(1)
                  << ", not 13" << std::endl;
        ret = false;
    }
    if (b->f<int>('a') != 'b') {
        std::cout << "test3, error: b->f<int>(char) = " << b->f<int>('a')
                  << ", not \'b\'" << std::endl;
        ret = false;
    }
    return ret;
}
}  // namespace test3

namespace test4 {
struct A;
struct B;
struct C;
struct base : enable_virtual_template_functions<A, B, C> {
    template <typename T>
    base(T ptr) : enable_virtual_template_functions(ptr) {}
    template <typename T>
    int f(int a) {
        constexpr auto base_f = [](base*, int) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }
    template <auto N>
    int f(int a) {
        constexpr auto base_f = [](base*, int) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(meta::reflect_constant(N), base_f, a);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B, C);
    }
    template <typename T>
    int f(char a) {
        constexpr auto base_f = [](base*, char) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }
    template <auto N>
    int f(char a) {
        constexpr auto base_f = [](base*, char) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(meta::reflect_constant(N), base_f, a);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }
    template <typename T>
    int f(int a, int b) {
        constexpr auto base_f = [](base*, int, int) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a, b);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B, C);
    }
    template <typename T>
    int f(int a, char b) {
        constexpr auto base_f = [](base*, int, char) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a, b);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }
    template <typename T>
    int f(char a, int b) {
        constexpr auto base_f = [](base*, char, int) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a, b);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }
    template <typename T>
    int f(char a, char b) {
        constexpr auto base_f = [](base*, char, char) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a, b);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B, C);
    }
    template <typename T>
    int f(int a, char b, double c, long d) {
        constexpr auto base_f = [](base*, int, char, double, long) {
            return 99;
        };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a, b, c, d);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A, B);
    }
};

struct A : base {
    A() : base(this) {}

    template <typename T>
    int f(int a) {
        return 11;
    }
    template <auto N>
    int f(int a) {
        return 12;
    }
    template <typename T>
    int f(char a) {
        return 13;
    }
    template <auto N>
    int f(char a) {
        return 14;
    }
    template <typename T>
    int f(int a, int b) {
        return 15;
    }
    template <typename T>
    int f(int a, char b) {
        return 16;
    }
    template <typename T>
    int f(char a, int b) {
        return 17;
    }
    template <typename T>
    int f(char a, char b) {
        return 18;
    }
    template <typename T>
    int f(int a, char b, double c, long d) {
        return 19;
    }
};

struct B : base {
    B() : base(this) {}

    template <typename T>
    int f(int a) {
        return 21;
    }
    template <auto N>
    int f(int a) {
        return 22;
    }
    template <typename T>
    int f(char a) {
        return 23;
    }
    template <auto N>
    int f(char a) {
        return 24;
    }
    template <typename T>
    int f(int a, int b) {
        return 25;
    }
    template <typename T>
    int f(int a, char b) {
        return 26;
    }
    template <typename T>
    int f(char a, int b) {
        return 27;
    }
    template <typename T>
    int f(char a, char b) {
        return 28;
    }
    template <typename T>
    int f(int a, char b, double c, long d) {
        return 29;
    }
};

struct C : base {
    C() : base(this) {}

    template <auto N>
    int f(int a) {
        return 32;
    }

    template <typename T>
    int f(int a, int b) {
        return 35;
    }

    template <typename T>
    int f(char a, char b) {
        return 38;
    }
};

#define test4_do(obj, func, val)                                         \
    if (obj->func != val) {                                              \
        std::cout << "test4, error: " #obj "-> " #func "= " << obj->func \
                  << ", not " #val << std::endl;                         \
        ret = false;                                                     \
    }

bool run() {
    bool ret = true;
    std::unique_ptr<base> a(std::make_unique<A>());
    std::unique_ptr<base> b(std::make_unique<B>());
    std::unique_ptr<base> c(std::make_unique<C>());

    test4_do(a, f<int>(1), 11);
    test4_do(a, f<1>(1), 12);
    test4_do(a, f<int>('a'), 13);
    test4_do(a, f<1>('a'), 14);
    test4_do(a, f<int>(1, 1), 15);
    test4_do(a, f<int>(1, 'a'), 16);
    test4_do(a, f<int>('a', 1), 17);
    test4_do(a, f<int>('a', 'a'), 18);
    test4_do(a, f<int>(1, 'a', 1.2, 4), 19);

    test4_do(b, f<int>(1), 21);
    test4_do(b, f<1>(1), 22);
    test4_do(b, f<int>('a'), 23);
    test4_do(b, f<1>('a'), 24);
    test4_do(b, f<int>(1, 1), 25);
    test4_do(b, f<int>(1, 'a'), 26);
    test4_do(b, f<int>('a', 1), 27);
    test4_do(b, f<int>('a', 'a'), 28);
    test4_do(b, f<int>(1, 'a', 1.2, 4), 29);

    test4_do(c, f<int>(1), 99);
    test4_do(c, f<1>(1), 32);
    test4_do(c, f<int>('a'), 99);
    test4_do(c, f<1>('a'), 99);
    test4_do(c, f<int>(1, 1), 35);
    test4_do(c, f<int>(1, 'a'), 99);
    test4_do(c, f<int>('a', 1), 99);
    test4_do(c, f<int>('a', 'a'), 38);
    test4_do(c, f<int>(1, 'a', 1.2, 4), 99);
    return ret;
}

}  // namespace test4

namespace test5 {
struct A;

template <typename T>
struct template_example {};

struct base : enable_virtual_template_functions<A> {
    template <typename T>
    base(T ptr) : enable_virtual_template_functions(ptr) {}

    template <typename T, int i, template <typename> typename tmp, char c,
              typename U>
    int f(int a) {
        constexpr auto base_f = [](base*, int) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION((^^T, meta::reflect_constant(i), ^^tmp,
                                        meta::reflect_constant(c), ^^U),
                                       base_f, a);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A);
    }
    template <typename T>
    int f(int a) {
        constexpr auto base_f = [](base*, char) { return 99; };
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, base_f, a);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(A);
    }
};

struct A : base {
    A() : base(this) {}

    template <typename T, int i, template <typename> typename temp, char c,
              typename U>
    int f(int a) {
        return 11;
    }

    template <typename T>
    int f(int a) {
        return 12;
    }
};

bool run() {
    bool ret = true;
    std::unique_ptr<base> a(std::make_unique<A>());

    if (a->f<int, 1, template_example, 'l', char>(1) != 11) {
        std::cout
            << "test5, error: a->f<int,1,template_example,'l',char>(int) = "
            << a->f<int, 1, template_example, 'l', char>(1) << ", not 11"
            << std::endl;
        ret = false;
    }
    if (a->f<int>(1) != 12) {
        std::cout << "test5, error: a->f<int>(int) = " << a->f<int>(1)
                  << ", not 12" << std::endl;
        ret = false;
    }

    return ret;
}
}  // namespace test5


namespace test6{
    struct A;
    struct B;
    struct C;

    struct base : enable_virtual_template_functions<A,B,C>{
        template<typename T>
        base(T* ptr) : enable_virtual_template_functions(ptr){}

        template<typename T>
        int f(){
            constexpr auto default_function = [](base*){return 99;};
            CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, default_function);
        }

        virtual int g(){
            return 99;
        }
    };

    struct A:base{
        template<typename T = A>
        A(T* ptr = 0):base(ptr){}

        template<typename T>
        int f(){
            return 11;
        }
        int g() override{
            return 21;
        }
    };

    struct B : A{
        template<typename T = B>
        B(T* ptr = 0):A(ptr){}

        template<typename T>
        int f(){
            return 12;
        }

        int g() override{
            return 22;
        }
    };

    struct C : B{
        C():B(this){}

        template<typename T>
        int f(){
            return 13;
        }

        int g() override{
            return 23;
        }
    };

    bool run(){
        bool ret = true;
        std::unique_ptr<base> a = std::make_unique<A>();
        std::unique_ptr<base> b = std::make_unique<B>();
        std::unique_ptr<base> c = std::make_unique<C>();
        if(a->f<int>() != 11){
            std::cout<<"test6, error: a->g() = "<<a->f<int>()<<", not 11"<<std::endl;
            ret = false;
        }
        if(a->g() != 21){
            std::cout<<"test6, error: a->g() = "<<a->g()<<", not 21"<<std::endl;
            ret = false;
        }
        if(b->f<int>() != 12){
            std::cout<<"test6, error: b->g() = "<<b->f<int>()<<", not 12"<<std::endl;
            ret = false;
        }
        if(b->g() != 22){
            std::cout<<"test6, error: b->g() = "<<b->g()<<", not 22"<<std::endl;
            ret = false;
        }
        if(c->f<int>() != 13){
            std::cout<<"test6, error: c->g() = "<<c->f<int>()<<", not 13"<<std::endl;
            ret = false;
        }
        if(c->g() != 23){
            std::cout<<"test6, error: c->g() = "<<c->g()<<", not 23"<<std::endl;
            ret = false;
        }

        return ret;
    }
}

bool test(bool prev, bool cur) {
    if (prev == true) {
        return cur;
    } else {
        return false;
    }
}

void run_tests() {
    bool all_good = true;
    all_good = test(all_good, test1::run());
    all_good = test(all_good, test2::run());
    all_good = test(all_good, test3::run());
    all_good = test(all_good, test4::run());
    all_good = test(all_good, test5::run());
    all_good = test(all_good, test6::run());
    if (!all_good) {
        std::cout << "test error occurred" << std::endl;
    }
}
}  // namespace VTF_tests



// to do
// test multiple base inherenatace
// test differeant return type overlaods