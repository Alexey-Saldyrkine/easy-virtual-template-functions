#pragma once
#include <meta>

namespace VTF {

namespace meta = std::meta;

namespace detail {

// A class that will store a list of reflection values. It will store the reflection of the template
// arguments for the virtual template function.
template <meta::info... TPs>
struct template_parameter_reflection_list {
    template_parameter_reflection_list() = delete;
};

// A class that will store a list of rtypes. It will store the types of the function arguments for
// the virtual template function.
template <typename... FPs>
struct function_parameter_type_list {
    function_parameter_type_list() = delete;
};

// This function converts its function arguments into a type_list of their types. This function is
// never called. The type_list type is obtained from a decltype statement.
template <typename... Args>
function_parameter_type_list<Args...> convert_args_to_type_list(Args... args);

// This function checks that the provided function reflection has the same function parameters as
// the provided list of parameters.
consteval bool are_function_parameters_the_same(meta::info func_refl,
                                                std::vector<meta::info> params) {
    unsigned int n = meta::parameters_of(func_refl).size();
    if (params.size() != n) {
        return false;
    }
    bool ret = true;
    for (unsigned int i = 0; i < n; i++) {
        if (params[i] != meta::type_of(meta::parameters_of(func_refl)[i])) {
            ret = false;
            break;
        }
    }
    return ret;
}

//to do -------------------------------
struct any_type_marker{
    any_type_marker()=delete;
};

consteval bool is_function_return_type_the_same(meta::info func_refl, meta::info ret_refl){
    if(ret_refl == ^^any_type_marker){
        return true;
    }else{
        return ret_refl == meta::return_type_of(func_refl);
    }
}

// This function is used to test that the overriding function of a derived class is a nonstatic
// member.
consteval bool is_nonstatic_member(meta::info refl) { return !meta::is_static_member(refl); }

// This class serves as the base template, which will be partially specialized to extract template
// parameter packs. It should never be instantiated. If it was instantiated, then make sure you used
// VTF::detail::type_list and VTF::detail::template_parameter_reflection_list.
template <meta::info class_refl, typename TP, typename FP, typename return_type = any_type_marker>
struct find_function_in_class {
    find_function_in_class() = delete;
    static_assert(false, "This class should never be instantiated. If it was instantiated, \
    then make sure you used VTF::detail::type_list and VTF::detail::template_parameter_reflection_list.");
};

// This class is a partially specialized class that will search the provided base class for a
// function template that accepts the provided template parameters and has the exact same function
// parameter types as the provided function argument type list.
template <meta::info class_refl, meta::info... TPs, typename... FPs, typename return_type>
struct find_function_in_class<class_refl, template_parameter_reflection_list<TPs...>,
                              function_parameter_type_list<FPs...>,return_type> {
    find_function_in_class() = delete;

    // This function accepts a string view of the name of the function that is searched for, and
    // will return that function’s reflection.
    static consteval meta::info get_func_refl(const std::string_view& name) {
        for (auto r : meta::members_of(class_refl, meta::access_context::current())) {
            if (is_nonstatic_member(r) && meta::is_function_template(r) && meta::can_substitute(r, {TPs...}) &&
            meta::has_identifier(r) && identifier_of(r) == name &&
                are_function_parameters_the_same(meta::substitute(r, {TPs...}), {^^FPs...})
                && is_function_return_type_the_same(meta::substitute(r, {TPs...}),^^return_type)) {
                return r;
            }
        }
        return ^^void;
    }

    // This function accepts a string view of the name of the function that is searched for, and
    // will return a reflection of the function with the template parameters substituted into it.
    static consteval meta::info get_function_sub(const std::string_view& name) {
        return meta::substitute(get_func_refl(name), {TPs...});
    }

    // This function accepts a string view of the name of the function that is searched for, and
    // will return a reflection of the return type of the function.
    static consteval meta::info get_return_type(const std::string_view& name) {
        return meta::return_type_of(get_function_sub(name));
    }
};

// This function searches for an overriding template function that accepts the same template and
// function parameters and has the same identifier.
template <meta::info func_name_refl, meta::info search_class_refl, typename template_parameters,
          typename function_parameters, typename return_type>
consteval meta::info find_overriding_tempalte_function() {
    using func_data =
        find_function_in_class<search_class_refl, template_parameters, function_parameters, return_type>;
    constexpr auto refl = func_data::get_func_refl(meta::identifier_of(func_name_refl));
    if constexpr (refl == ^^void) {
        return ^^void;
    } else {
        return func_data::get_function_sub(meta::identifier_of(func_name_refl));
    }
}



// This concept is used to test that the overriding function of a derived class can be called with
// the provided arguments.
template <meta::info func, typename base_type, typename... Args>
concept is_overriding_function_callable_with_args = requires(base_type* ptr, Args... args) {
    { ptr->[:func:](std::forward<Args>(args)...) };
};

// This concept is used to test that the overriding function of a derived class returns a type that
// is the same as the provided return type.
template <meta::info func, typename base_type, typename return_type, typename... Args>
concept is_overriding_function_return_type_correct = requires(base_type* ptr, Args... args) {
    { ptr->[:func:](std::forward<Args>(args)...) } -> std::same_as<return_type>;
};

// This function checks if an overriding function was found.
consteval bool does_not_have_overriding_function(meta::info r) { return r == ^^void; }

// This concept is used to test that the overriding function of a derived class is a nonstatic
// member that can be called with the provided arguments and that it returns a type that is the same
// as the provided return type.
template <meta::info func, typename T, typename return_type, typename... Args>
concept overriding_template_function_requirments =
    does_not_have_overriding_function(func) ||
    (is_nonstatic_member(func) && is_overriding_function_callable_with_args<func, T, Args...> &&
     is_overriding_function_return_type_correct<func, T, return_type, Args...>);

// This function is the intermediate function that will find and call the overriding template
// function, if there is one, or call the base function, if there isn’t.
template <typename base_type, typename return_type, meta::info func_name_refl,
          typename searched_class, typename template_parameters, typename function_parameter_list,
          auto base_function, typename... Args>
    requires overriding_template_function_requirments<
        find_overriding_tempalte_function<func_name_refl, ^^searched_class, template_parameters,
                                          function_parameter_list,return_type>(),
        searched_class, return_type, Args...>
return_type interm_func(base_type* ptr, Args&&... args) {
    constexpr auto found_func_refl =
        find_overriding_tempalte_function<func_name_refl, ^^searched_class, template_parameters,
                                          function_parameter_list,return_type>();
    if constexpr (found_func_refl != ^^void) {
        return static_cast<searched_class*>(ptr)->[:found_func_refl:](std::forward<Args>(args)...);
    } else {
        return base_function(ptr, std::forward<Args>(args)...);
    }
}

// This class creates an array of function pointers to an intermediate function for each of the
// derived classes. This array serves as a virtual function table. The provided index argument is
// used to call the appropriate overriding function. The index refers to the Ith derived class of
// the provided list of derived classes.
template <typename base_type, meta::info func_name_refl, typename template_parameters,
          typename function_parameter_list, auto base_function, typename... Ts>
struct template_func_vtable {
    template_func_vtable() = delete;
    static constexpr int N = sizeof...(Ts);
    template <typename return_type, typename... Args>
    static auto call_function(base_type* ptr, int index, Args&&... args) {
        using func_type = return_type (*)(base_type*, Args...);
        static constexpr std::array<func_type, N> vtable = {
            (&interm_func<base_type, return_type, func_name_refl, Ts, template_parameters,
                          function_parameter_list, base_function, Args...>)...};
        return vtable[index](ptr, std::forward<Args>(args)...);
    }
};

// This function converts a template pack into a vector of reflections.
template <typename... Ts>
consteval std::vector<meta::info> type_pack_to_info_vec() {
    return {^^Ts...};
}

// This function returns true if the type T is present in the parameter pack Ts.
template <typename T, typename... Ts>
consteval bool is_T_in_Ts() {
    for (auto r : std::vector<meta::info>({(^^Ts)...})) {
        if (^^T == r) {
            return true;
        }
    }
    return false;
}

// This function will return an integer, which is the zero-indexed position of T in the template
// pack Ts.
template <typename T, typename... Ts>
    requires(is_T_in_Ts<T, Ts...>())
consteval unsigned int get_index_of_T_in_Ts() {
    for (unsigned int i = 0; i < sizeof...(Ts); i++) {
        if (type_pack_to_info_vec<Ts...>()[i] == ^^T) {
            return i;
        }
    }
    return -1;  // impossible case
}

// This concept checks that the given function can be called with the given arguments.
template <auto func, typename base_type, typename... Args>
concept is_function_callable_with_given_args = requires(base_type* ptr, Args... args) {
    { func(ptr, std::forward<Args>(args)...) };
};

// This concept checks that the given function returns a value of a given type.
template <auto func, typename base_type, typename return_type, typename... Args>
concept is_function_return_type_correct = requires(base_type* ptr, Args... args) {
    { func(ptr, std::forward<Args>(args)...) } -> std::same_as<return_type>;
};

// This concept is used to make sure that the provided default function, which is called when a
// derived class does not override the virtual template function, can be called with the provided
// arguments and returns a value of the same type as the virtual template function.
template <auto func, typename base_type, typename return_type, typename... Args>
concept defualt_function_requirments =
    is_function_callable_with_given_args<func, base_type, Args...> &&
    is_function_return_type_correct<func, base_type, return_type, Args...>;

// This concept is used by the constructor of enable_virtual_template_functions, and therefore the
// constructor of the base class, to test if the type of the given pointer is one of the declared
// derived classes of the base.
template <typename T, typename... Ts>
concept is_T_derived_from_base = is_T_in_Ts<T, Ts...>();

// This function checks that for the provided derived class, an overriding function has been found.
template <meta::info func, typename template_parameters, typename function_parameters, typename return_type,
          typename derivedT>
consteval bool does_type_have_overriding_function() {
    return find_overriding_tempalte_function<func, ^^derivedT, template_parameters,
                                             function_parameters,return_type>() != ^^void;
}

// This function checks that the reflection for the base class's virtual template function has been
// found.
consteval bool this_func_refl_has_been_found(meta::info func) { return func != ^^void; }

// This concept requires that the reflection for the base class's virtual template function and an
// overriding function have been found.
template <meta::info func, typename template_parameters, typename function_parameters, typename return_type,
          typename... Ts>
concept require_override_for_template_function_for_all_types =
    this_func_refl_has_been_found(func) &&
    ((does_type_have_overriding_function<func, template_parameters, function_parameters,return_type, Ts>()) &&
     ...);



}  // namespace detail
/*
The enable_virtual_template_functions class enables virtual template functions.

The classes template parameter pack Ts will be the derived types that will contain the overriding
template functions. This class must be inherited by the base class, which will be inherited by the
derived types. The constructor of enable_virtual_template_functions requires a pointer to a derived
class. The type of the pointer will be used to determine what value type_index will be set to. The
type must be included in the template parameter pack Ts. type_index is a value between 0 and
sizeof...(Ts). It refers to the Ith type in the parameter pack Ts. When a virtual template function
is called, type_index will be used to determine which overriding function to call. Type_index is a
private data member and should never be manually changed, as this can cause undefined behaviour or
crashes.

The protected member function call_virtual_template_function should never be manually called.
Instead, you should use the macro CALL_VIRTUAL_TEMPLATE_FUNCTION in the base class. This macro will
auto-fill a lot of the template parameters that the function requires. When this function is called
from a base class, it will receive the pointer to this base class, type_index, and the provided
function arguments. Depending on the type_index, the base pointer will be cast to the appropriate
derived class, and the overriding template function will be called with the same template parameters
and function arguments as it was called from the base.

The public member function is_base_pointer_of can be used to determine the value of type_index, and
which derived class the base is part of at runtime.
*/
template <typename... Ts>
class enable_virtual_template_functions {
   private:
    int derived_type_index;

   protected:
    template <typename base_type, meta::info func_refl, typename parameterList,
              typename function_parameter_list, auto default_function>
    using vtable = detail::template_func_vtable<base_type, func_refl, parameterList,
                                                function_parameter_list, default_function, Ts...>;

    template <typename T>
        requires detail::is_T_derived_from_base<T, Ts...>
    enable_virtual_template_functions(T*)
        : derived_type_index(detail::get_index_of_T_in_Ts<T, Ts...>()) {}

    template <typename base_type, meta::info func_refl, typename paramaterList,
              typename function_parameter_list, auto default_function, typename return_type,
              typename... Args>
        requires detail::defualt_function_requirments<default_function, base_type, return_type,
                                                      Args...>
    constexpr return_type call_virtual_template_function(base_type* ptr, Args&&... args) {
        return vtable<base_type, func_refl, paramaterList, function_parameter_list,
                      default_function>::template call_function<return_type>(ptr,
                                                                             derived_type_index,
                                                                             std::forward<Args>(
                                                                                 args)...);
    }

   public:
    template <typename T>
        requires detail::is_T_derived_from_base<T, Ts...>
    constexpr bool is_base_of() {
        return derived_type_index == detail::get_index_of_T_in_Ts<T, Ts...>();
    }

    enable_virtual_template_functions() = delete;
};

}  // namespace VTF

// Macros to remove parentheses from a macro argument, if there are any.
#define VTF_EXTRACT(...) VTF_EXTRACT __VA_ARGS__
#define VTF_NOTHING_VTF_EXTRACT
#define VTF_PASTE(x, ...) x##__VA_ARGS__
#define VTF_EVALUATING_PASTE(x, ...) VTF_PASTE(x, __VA_ARGS__)
#define VTF_UNPAREN(...) VTF_EVALUATING_PASTE(VTF_NOTHING_, VTF_EXTRACT __VA_ARGS__)

/*
This macro is the primary way of calling a virtual template function. This macro should only be
called inside a function of the base class that is derived from
VTF::enable_virtual_template_functions. That function will be the virtual base function. This macro
automatically collects information about this function, specifically its identifier, return type,
and the class that contains the base virtual function. It also wraps the template and function
parameters into the appropriate types. This can be done manually, but it is not recommended.

This macro accepts three macro arguments. The first is the template parameters of the virtual
template function. If there is more than one template parameter or a parameter contains a comma, you
need to put them in parentheses. There needs to be at least one template parameter. They need to be
in the same order as they appear in the base virtual function’s declaration. The second is the
default function that will be called if there isn’t an overriding function in a derived class. The
default function needs its first function argument to be a pointer to the base class. The default
function can be any callable object that can be a non-type template parameter. The other parameters
must be exactly the same as the base virtual function’s. If there is a comma in the default
function, it needs to be parenthesised. The third argument is the function parameters of the base
virtual function. They need to be in the same order as they appear in the base virtual function’s
declaration.

This macro will call the appropriate virtual function and get its return value. After this macro,
the function terminates and returns the value.
*/
#define CALL_VIRTUAL_TEMPLATE_FUNCTION(TEMPLATE_PARAMETERS, DEFUALT_FUNCTION, ...)              \
    using VTF_base_class_type = std::remove_pointer_t<decltype(this)>;                          \
    using VTF_template_parameter_list =                                                         \
        VTF::detail::template_parameter_reflection_list<VTF_UNPAREN(TEMPLATE_PARAMETERS)>;      \
    using VTF_function_parameter_list =                                                         \
        decltype(VTF::detail::convert_args_to_type_list(VTF_UNPAREN(__VA_ARGS__)));             \
    using VTF_func_data =                                                                       \
        VTF::detail::find_function_in_class<^^VTF_base_class_type, VTF_template_parameter_list, \
                                            VTF_function_parameter_list>;                       \
    constexpr auto VTF_this_func_refl = VTF_func_data::get_func_refl(__func__);                 \
    static_assert(                                                                              \
        VTF::detail::this_func_refl_has_been_found(VTF_this_func_refl),                         \
        "Did not find this function reflection. Did you enter the macro arguments properly?");  \
    using VTF_return_type = [:VTF_func_data::get_return_type(__func__):];                       \
    return VTF_base_class_type::enable_virtual_template_functions::                             \
        template call_virtual_template_function<                                                \
            VTF_base_class_type, VTF_this_func_refl, VTF_template_parameter_list,               \
            VTF_function_parameter_list, VTF_UNPAREN(DEFUALT_FUNCTION), VTF_return_type>(       \
            this __VA_OPT__(, ) VTF_UNPAREN(__VA_ARGS__));

// This macro creates an assertion that the provided derived classes have an overriding function.
// This macro should only be used after the CALL_VIRTUAL_TEMPLATE_FUNCTION macro.
#define VTF_ASSERT_OVERRIDE_FOR_TYPES(...)                                                      \
    static_assert(VTF::detail::require_override_for_template_function_for_all_types<            \
                  VTF_this_func_refl, VTF_template_parameter_list, VTF_function_parameter_list, VTF_return_type,\
                  __VA_ARGS__>, "One or more required types did not have an overrding template function");
// end of VTF
