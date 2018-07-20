// classes for using coroutines with Qt signals and slots
// The goal: from within a slot, co_await on a one-shot signal

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

#include "meta.hpp"

namespace qtcoro {

//
// Special TS-required types
//

// first, the type "returned" by (more accurately, "supplied by the call expression from") our coroutines
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
        // we inherit it from promise_base

        return_object get_return_object() {
            return return_object(*this);
        }

        void unhandled_exception() {}  // do nothing :)

    };

private:
    std::experimental::coroutine_handle<promise_type> m_coro;

};

// that specialization for void values
template<>
template<>
struct return_object<void>::promise_base<void> {
    auto return_void() const noexcept {
        return std::experimental::suspend_always(); // ?? not sure
    }
};

//
// Slot "factory"
// Gives us something to connect() signals to, that integrates with coroutines
//

template<typename Result, typename... Args>
struct make_slot;

// for two or more arguments we supply a std::tuple from the awaiter
template<typename... Args>
struct make_slot<std::tuple<Args...>, Args...> {
    using Result = std::tuple<Args...>;
    auto operator()(QMetaObject::Connection& signal_conn,
                    Result& result,
                    std::experimental::coroutine_handle<>& coro_handle) {
        return [&signal_conn, &coro_handle, &result]
            (Args... a) {
            // all our awaits are one-shot, so we immediately disconnect
            QObject::disconnect(signal_conn);
            // put the result where the awaiter can supply it from await_resume()
            result = std::make_tuple(a...);
            // resume execution inside the coroutine at the co_await
            coro_handle.resume();
        };
    }
};

// for a single argument it's just that particular type
template<typename Arg>
struct make_slot<Arg, Arg> {
    auto operator()(QMetaObject::Connection& signal_conn,
                    Arg& result,
                    std::experimental::coroutine_handle<>& coro_handle) {
        return [&signal_conn, &coro_handle, &result]
            (Arg a) {
            QObject::disconnect(signal_conn);
            result = a;
            coro_handle.resume();
        };
    }
};

// no argument - result is void, don't set anything
template<>
struct make_slot<void>
{
    auto operator()(QMetaObject::Connection& signal_conn,
                    std::experimental::coroutine_handle<>& coro_handle) {
        return [&signal_conn, &coro_handle]() {
            QObject::disconnect(signal_conn);
            coro_handle.resume();
        };
    }
};

//
// Awaitable class
//

// first, a special base to handle possibly nullary signals
template<typename Derived, typename Result>
struct awaitable_signal_base {
    // not nullary - we have a real value to set when the signal arrives
    template<typename Object, typename... Args>
    awaitable_signal_base(Object* src, void (Object::*method)(Args...),
                          std::experimental::coroutine_handle<>& coro_handle) {
        signal_conn_ =
            QObject::connect(src, method,
                             make_slot<Result, Args...>()(signal_conn_, derived()->signal_args_, coro_handle));

    }

    Derived * derived() { return static_cast<Derived*>(this); }

protected:
    Result signal_args_;
    QMetaObject::Connection signal_conn_;
};

template<typename Derived>
struct awaitable_signal_base<Derived, void> {
    // nullary, i.e., no arguments to signal and nothing to supply to co_await
    template<typename Object, typename... Args>
    awaitable_signal_base(Object* src, void (Object::*method)(Args...),
                          std::experimental::coroutine_handle<>& coro_handle) {
        // hook up the slot version that doesn't try to store signal args
        signal_conn_ = QObject::connect(src, method,
                                        make_slot<void>()(signal_conn_, coro_handle));
    }

protected:
    QMetaObject::Connection signal_conn_;
    // no need to store signal arguments since there are none
};


// forward reference for our "signal args -> co_await result" TMP code
template<typename F>
struct signal_args_t;

// The rest of our awaitable
template<typename Signal, typename Result = typename signal_args_t<Signal>::type>
struct awaitable_signal : awaitable_signal_base<awaitable_signal<Signal, Result>, Result> {
    using obj_t    = typename member_fn_t<Signal>::cls_t;

    awaitable_signal(obj_t * src, Signal method)
        : awaitable_signal_base<awaitable_signal, Result>(src, method, coro_handle_) {}

    struct awaiter {
        awaiter(awaitable_signal * awaitable) : awaitable_(awaitable) {}

        bool await_ready() const noexcept {
            return false;  // we are waiting for the signal to arrive
        }

        template<typename P>
        void await_suspend(std::experimental::coroutine_handle<P> handle) noexcept {
            // we have now been suspended but are able to do something before
            // returning to caller-or-resumer
            // such as storing the coroutine handle!
            awaitable_->coro_handle_ = handle;    // store for later resumption
        }

        template<typename R = Result>
        typename std::enable_if_t<!std::is_same_v<R, void>, R>
        await_resume() noexcept {
            return awaitable_->signal_args_;
        }

        template<typename R = Result>
        typename std::enable_if_t<std::is_same_v<R, void>, void>
        await_resume() noexcept {}

    private:
        awaitable_signal* awaitable_;

    };

    awaiter operator co_await () { return awaiter{this}; }

private:
    std::experimental::coroutine_handle<> coro_handle_;

};

template<typename T, typename F>
awaitable_signal<F>
make_awaitable_signal(T * t, F fn) {
    return awaitable_signal<F>{t, fn};
}

//
// some light metaprogramming
//

// deduce the type we want to return from co_await from the signal's signature
// result of co_await should be void, one value, or a tuple of values
// depending on how many parameters the signal has

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

//
// Use a simple predicate and the filter metafunction
// to make a list of non-empty types
//

template<typename T>
using not_empty = std::negation<std::is_empty<T>>;

template<typename Sequence>
struct filter_empty_types {
    using type = typename filter<not_empty, Sequence>::type;
};

// now put it all together:

template<typename F>
struct signal_args_t {
    // get argument list
    using args_t = typename member_fn_t<F>::arglist_t;
    using classname_t = typename member_fn_t<F>::cls_t;
    // remove any empty (including "QPrivateSignal") parameters from the list
    using no_empty_t = typename filter_empty_types<args_t>::type;
    // apply std::decay_t to all arg types
    using decayed_args_t = typename apply_to_tuple<std::decay_t, no_empty_t>::type;
    // special case 0 and 1 argument
    using type = typename special_case_tuple<decayed_args_t>::type;
};

}

#endif  // QTCORO_HPP
