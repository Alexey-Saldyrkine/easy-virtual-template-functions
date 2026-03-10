#include <cassert>
#include <iostream>
#include <memory>

#include "VTF_tests.cpp"
#include "virtual_template_functions.hpp"
using namespace std::meta;
using namespace std;

// Derived classes must be forward declared
struct D1;
struct D2;
struct D3;

// The base class. It will inherit the type 'enable_virtual_template_functions'
// with the wanted derived classes as it's template arguments
struct Base : VTF::enable_virtual_template_functions<D1, D2, D3> {
    // The Base must provide a pointer of a derived type to the constructor of
    // 'enable_virtual_template_functions'. This pointer should be of the
    // derived type that this base is constructed from.
    template <typename DerivedT>
    Base(DerivedT* ptr) : enable_virtual_template_functions(ptr) {}

    // The base virtual template function. It is declared like a normal template
    // function.
    template <typename T, auto N>
    int func(int a, char b) {
        // The default function that will be used if a derived class has no
        // overriding function.
        constexpr auto default_function = [](Base* ptr, int a, char b) {
            return 99;
        };

        // This macro will case the 'this' pointer to the correct derived type
        // and call the correct overriding function. It will then return the
        // result, ending the function.
        CALL_VIRTUAL_TEMPLATE_FUNCTION((^^T, meta::reflect_constant(N)),
                                       default_function, a, b);
        // This macro creates a static_assert that only passes if D1 and D2 have
        // overriding functions.
        VTF_ASSERT_OVERRIDE_FOR_TYPES(D1, D2);
    }

    // Default functions also be a static member function.
    static int default_function_2(Base* ptr, int a, double b, char c) {
        return 99;
    }

    template <typename T>
    int func2(int a, double b, char c) {
        // Function arguments in the macro can be either parenthesized or not.
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T, default_function_2, (a, b, c));
        VTF_ASSERT_OVERRIDE_FOR_TYPES(D2, D3);
    }
};

// The first derived class
struct D1 : Base {
    // The constructor of Base needs the 'this' pointer.
    D1() : Base(this) {}

    // The overriding function is declared like a normal function.
    template <typename T, auto N>
    int func(int a, char b) {
        return 11;
    }
};

struct D2 : Base {
    // It is not necessary to use the 'this' pointer, but it is recommended, as
    // using the wrong pointer type could cause an error. Only the type of the
    // pointer matters. The value is never used.
    D2() : Base(static_cast<D2*>(0)) {}

    template <typename T, auto N>
    int func(int a, char b) {
        return 12;
    }

    template <typename T>
    int func2(int a, double b, char c){
        return 22;
    }
};

struct D3 : Base {
    D3() : Base(this) {}

    template <typename T>
    int func2(int a, double b, char c) {
        return 23;
    }
};



int main() {
    using Base_ptr_T = std::unique_ptr<Base>;

    std::vector<Base_ptr_T> objs;

    objs.push_back(std::make_unique<D1>());
    objs.push_back(std::make_unique<D2>());
    objs.push_back(std::make_unique<D3>());

    assert((objs[0]->func<int, 2>(1, 'a') == 11));
    assert((objs[1]->func<int, 2>(1, 'a') == 12));
    assert((objs[2]->func<int, 2>(1, 'a') == 99));

    assert((objs[0]->func2<int>(1, 1.0, 'a') == 99));
    assert((objs[1]->func2<int>(1, 1.0, 'a') == 22));
    assert((objs[2]->func2<int>(1, 1.0, 'a') == 23));

    VTF_tests::run_tests();
}
//