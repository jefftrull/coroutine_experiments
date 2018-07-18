// classes for using coroutines with Qt signals and slots
// The goal: from within a slot, co_await on a one-shot signal
// fine if that code is inside a lambda or whatever

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

#ifndef QTCORO_HPP
#define QTCORO_HPP

#include <iostream>
#include <experimental/coroutine>
#include <QObject>

namespace qtcoro {

// first, the type "returned" (more accurately, "supplied by the call expression") by our coroutines
template<typename T=void>
struct return_object {
    struct promise_type;
    return_object(promise_type & p)
        : m_coro(std::experimental::coroutine_handle<promise_type>::from_promise(p)) {}

    return_object(return_object const &) = delete;
    return_object(return_object && other) {
        m_coro = other.m_coro;
        other.m_coro = nullptr;
    }

    ~return_object() {
        if (m_coro) {
            m_coro.destroy();
        }
    }

    // promise type must have either return_void or return_value member but not both
    // not even if one is SFINAEd out - you cannot have both names present, per Lewis Baker
    // So this is the fix: we specialize the base and inherit
    template<typename U>
    struct promise_base {
        auto return_value(U const&) const noexcept {
            return std::experimental::suspend_always(); // ?? not sure
        }
    };
    // void specialization to replace with return_void() is below at namespace scope

    struct promise_type : promise_base<T> {
        // coroutine promise requirements:

        auto initial_suspend() const noexcept {
            return std::experimental::suspend_never(); // produce at least one value
        }

        auto final_suspend() const noexcept {
            return std::experimental::suspend_always(); // ?? not sure
        }

        // either return_void or return_value will exist, depending on T

        return_object get_return_object() {
            return return_object(*this);
        }

        void unhandled_exception() {}  // do nothing :)

    };


private:
    std::experimental::coroutine_handle<promise_type> m_coro;

};

template<>
template<>
struct return_object<void>::promise_base<void> {
    auto return_void() const noexcept {
        return std::experimental::suspend_always(); // ?? not sure
    }
};

template<typename F>
struct member_fn_t;

template<typename F>
struct signal_args_t;

// We also need something to wrap
template<typename Signal>
struct awaitable_signal {
    using result_t = typename signal_args_t<Signal>::type;
    using obj_t    = typename member_fn_t<Signal>::cls_t;
    using args_t   = typename member_fn_t<Signal>::arglist_t;

    awaitable_signal(obj_t * src, Signal method)  // handle multiple later
    {
        // hook up the signal to a custom-made function
        // that function should:
        // - do whatever is required to maintain our lifetime
        signal_conn_ = QObject::connect(src, method,
                                        [=]() {   // need a different signature for void!
                                            // disconnect the signal
                                            QObject::disconnect(signal_conn_);
                                            // arrange to deliver args to co_await caller
                                            // get them from awaitable_

                                            // we must lifetime extend in this lambda
                                            // or do we? Storing coro handle may be enough
                                            // OK but... when we create this closure the coro handle is unknown
                                            // we have "this"

                                            // resume
                                            coro_handle_.resume();

                                            // when we get here, either we hit another co_await or the
                                            // original coroutine has completed
                                            // that's what is meant by resumer-or-caller
                                            // we are the resumer, the original code is the caller
                                            // I think...
                                        });
    }

    struct awaiter
    {
        awaiter(awaitable_signal * awaitable) : awaitable_(awaitable) {}

        bool await_ready() const noexcept
        {
            return false;  // we are waiting for the signal to arrive
        }

        template<typename P>
        void await_suspend(std::experimental::coroutine_handle<P> handle) noexcept
        {
            awaitable_->coro_handle_ = handle;    // store for later resumption
            // we have now been suspended but are able to do something before
            // returning to caller-or-resumer
            // such as storing the coroutine handle!
        }

        result_t await_resume() noexcept {    // check on "noexcept"
            // We have been resumed, presumably because the signal we were waiting for has arrived

            // disconnect the signal here?

            // and now we resume

        }

        awaitable_signal* awaitable_;

    };

    awaiter operator co_await () { return awaiter{this}; }

private:
    QMetaObject::Connection signal_conn_;
    std::experimental::coroutine_handle<> coro_handle_;

};

// BOZO maybe use deduction guides?
template<typename T, typename F>
awaitable_signal<F>
make_awaitable_signal(T * t, F fn) {
    return awaitable_signal<F>{t, fn};
}

// deduce the type we want to return from co_await from the signal's signature
// result of co_await should be void, one value, or a tuple of values
// depending on how many parameters the signal has

// BOZO create a utility metafunction file

template<typename C, typename... Args>
struct member_fn_t<void (C::*)(Args...)> {
    using arglist_t = std::tuple<Args...>;
    using cls_t = C;
};


// can I make everything from the base template?
template<template<typename> class MF,
         typename Tuple>
struct apply_to_tuple;

template<template<typename> class MF,
         typename... Elements>
struct apply_to_tuple<MF, std::tuple<Elements...>> {
    using type = std::tuple<MF<Elements>...>;
};


// produce void or T for small tuples
template<typename T>
struct special_case_tuple {
    using type = T;
};

// just one type
template<typename T>
struct special_case_tuple<std::tuple<T>> {
    using type = T;
};

// empty list
template<>
struct special_case_tuple<std::tuple<>> {
    using type = void;
};

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

template<typename T>
using not_empty = std::negation<std::is_empty<T>>;

template<typename Sequence>
struct filter_empty_types {
    using type = typename filter<not_empty, Sequence>::type;
};


// remarkably decltype(&T::M) does just fine, I don't know why
template<typename F>
struct signal_args_t {
    // get argument list
    using args_t = typename member_fn_t<F>::arglist_t;
    using classname_t = typename member_fn_t<F>::cls_t;
    // remove any QPrivateSignal parameters from the end
    using no_empty_t = typename filter_empty_types<args_t>::type;
    // apply std::decay_t to all arg types
    using decayed_args_t = typename apply_to_tuple<std::decay_t, no_empty_t>::type;
    // special case 0 and 1 argument
    using type = typename special_case_tuple<decayed_args_t>::type;
};

}

#endif  // QTCORO_HPP
