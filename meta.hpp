// Metaprogramming utilities for my coroutine stuff
/*
Copyright (c) 2018 Jeff Trull <edaskel@att.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef UTIL_HPP
#define UTIL_HPP

#include <tuple>
#include <type_traits>

//
// Extract object type, return type, and parameters from member function
//

template<typename F>
struct member_fn_t;

template<typename R, typename C, typename... Args>
struct member_fn_t<R (C::*)(Args...)> {
    using arglist_t = std::tuple<Args...>;
    using cls_t = C;
    using ret_t = R;
};

//
// Apply a template template "function" to all types in a std::tuple
//

template<template<typename> class MF,
         typename Tuple>
struct apply_to_tuple;

template<template<typename> class MF,
         typename... Elements>
struct apply_to_tuple<MF, std::tuple<Elements...>> {
    using type = std::tuple<MF<Elements>...>;
};

//
// Filter types in a std::tuple by a template template "predicate"
//

template<template<typename> class Predicate,
         typename Sequence>
struct filter;

template<template<typename> class Predicate,
         typename Head,
         typename... Elements>
struct filter<Predicate, std::tuple<Head, Elements...>> {
    using type=std::conditional_t<Predicate<Head>::value,
                                  decltype(std::tuple_cat(std::declval<std::tuple<Head>>(),
                                                          std::declval<typename filter<Predicate, std::tuple<Elements...>>::type>())),
                                  std::tuple<Elements...>>;
};

// terminate recursion
template<template<typename> class Predicate>
struct filter<Predicate, std::tuple<>> {
    using type = std::tuple<>;
};

#endif  // UTIL_HPP
