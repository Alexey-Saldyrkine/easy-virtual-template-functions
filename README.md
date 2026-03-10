# Easy virtual template functions
Easy to use, single line, virtual template functions. C++26

## Example of use:
```cpp
struct D1;
struct D2;

struct Base: VTF::enable_virtual_template_functions<D1,D2>{
    template<typename T>
    Base(T* ptr): enable_virtual_template_functions(ptr){}

    template<typename T>
    int f(int a){
        constexpr auto default_function = [](Base* ptr, int a){ return 99;};
        CALL_VIRTUAL_TEMPLATE_FUNCTION(^^T,default_function,a);
        VTF_ASSERT_OVERRIDE_FOR_TYPES(D1);
    }
};

struct D1:Base{
    D1():Base(this){}

    template<typename T>
    int f(int a){
        return 11;
    }
};

struct D2:Base{
    D2():Base(this){}
    
    template<typename T>
    int f(int a){
        return 12;
    }
};

int main(){
    using PtrT = std::unique_ptr<Base>;
    PtrT a = std::make_unique<D1>();
    PtrT b = std::make_unique<D2>();
    assert((a->f<int>(1) == 11));
    assert((b->f<int>(1) == 12));
}
```

## How to make virtual template functions
### Declaring base and derived classes
When using virtual functions, there are two types of classes: the base class and a derived class. The base class has the base virtual template function. The derived class has an overriding template function. When you call the base virtual template function, it invokes the matching overriding function. Both use the same template and function parameters. The result of the overriding function is returned by the base function. 

The base class must inherit from the ‘enable_virtual_template_functions’ class. This class uses a type parameter pack as its template parameter. All derived classes must be included in this parameter pack. As a result, derived classes must be forward declared. If a class is not in the parameter pack, it will not be treated as a derived class for this specific base class. The base virtual template function will not search the class for an overriding function, and it will never call any functions from this class.

The constructor of ‘enable_virtual_template_functions’ is a templated constructor that takes a pointer to a derived type. The value of the pointer can be any value, as it is never used. Only the type of the pointer matters. The type of the pointer can only be one of the derived classes for that specific base class. When a base virtual template function is called, the corresponding overriding template function of the derived type of the pointer will be invoked, if one exists. If not, then the default function will be called.

Since the ‘enable_virtual_template_functions’ constructor requires a derived class pointer, the constructor of the base class should also be templated with a function parameter being the derived pointer.

The derived class will inherit the base class. The constructor of the derived class should pass the ‘this’ pointer to the base class constructor, as it is automatically a pointer of the derived type. Alternatively, you could cast an arbitrary integer value to a pointer to the derived type. The effect is the same because the value of the pointer is not used.

Example of declaring base and derived classes:
```cpp
struct Derived1;
struct Derived2;

struct Base: VTF::enable_virtual_template_functions<Derived1,Derived2>{
    template<typename T>
    Base(T ptr):enable_virtual_template_functions(ptr){}
};

struct Derived1:Base{
    Derived1():Base(this){}
};

struct Derived2:Base{
    Derived2():Base(static_cast<Derived2*>(0)){}
};
```
### Declaring the virtual functions

There are two types of virtual template functions: the base virtual template function and the overriding function. The base virtual function will cast the base class object to the appropriate derived class and call the appropriate overriding function. The base virtual template function is a member of the base class. The overriding virtual template function is a member of the derived class.

A base virtual template function is declared like a normal member template function. To turn it from a normal to a base virtual template function, the function must call the macro ‘CALL_VIRTUAL_TEMPLATE_FUNCTION’. This macro automatically gathers information on this function and calls a protected member function of ‘enable_virtual_template_functions’, which calls the appropriate overriding function of the derived class and returns its result.

The macro has three macro parameters: template parameter reflections, the default function, and function parameters. The template parameters reflections will be the template parameters that the overriding template function will be instantiated with. The default function will be the function called if no overriding function is found. The function parameters will be the function arguments that the overriding function will be called with. The function parameters are forwarded. 

If a macro parameter requires a comma, the macro parameters must be parenthesized. 

The macro parameters for the template parameter reflections and function parameters must be given fully and in the same order as they appear in the function declaration. If they aren’t, then a compilation error will occur with the message saying that the base virtual template function could not be found.  

The overriding virtual template function is declared as a normal member template function. No changes are needed.

When a base virtual template function is instantiated, it searches for an overriding function for each of the declared derived classes. For each nonstatic member of a derived class, it searches for the first member function that satisfies all of the following conditions:

- The member function is a template function, and it can be instantiated with the same template parameters as the base virtual template function.
- The member function has an identifier, which is the same as the identifier of the base virtual template function, meaning they share the same name.
- The member function, instantiated with the template parameters, has the same function parameter types as the base virtual template function.
- The member function, instantiated with the template parameters, has the same return type as the base virtual template function.

When a base virtual template function is called, one of two functions will be called. In the case that an overriding function has been found, the ‘this’ pointer will be static cast to the appropriate derived type, and the derived class's overriding function will be called with the same template parameters, and the function parameters are forwarded from the base virtual template function. In the case that an overriding function has not been found, the default function will be called with the first function parameter being the ‘this’ pointer of the base class type, and the rest will be the function parameters of the base virtual template function forwarded. The results of these functions are then returned by the base virtual template function.

Example of declaring virtual template functions:
```cpp
struct Derived1;
struct Derived2;

struct Base: VTF::enable_virtual_template_functions<Derived1,Derived2>{
    template<typename T>
    Base(T ptr):enable_virtual_template_functions(ptr){}

    // declaring a base virtual template function
    template<typename T, typename U, int N>
    int f(int a, T b){
        // declaring the default function
        constexpr auto default_function = [](Base* ptr, int a, T b){
            return 99;
        };
        // calling te macro with te proper parameters
        CALL_VIRTUAL_TEMPLATE_FUNCTION((^^T,^^U,meta::reflect_constant(N)),default_function,a,b);
    }
};

struct Derived1:Base{
    Derived1():Base(this){}

    // declaring an overriding virtual template function
    template<typename T, typename U, int N>
    int f(int a, T b){
        return 11;
    }
};

struct Derived2:Base{
    Derived2():Base(static_cast<Derived2*>(0)){}

    // declaring an overriding virtual template function
    template<typename T, typename U, int N>
    int f(int a, T b){
        return 12;
    }
};
```