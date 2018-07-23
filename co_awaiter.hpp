// A basic return object and promise type for a coroutine that co_awaits
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

#ifndef CO_AWAITER_HPP
#define CO_AWAITER_HPP

#include <type_traits>
#include <experimental/coroutine>

template<typename T=void>
struct await_return_object {
    struct promise_type;
    await_return_object(promise_type & p) : m_coro(std::experimental::coroutine_handle<promise_type>::from_promise(p)) {}

    await_return_object(await_return_object const &) = delete;
    await_return_object(await_return_object && other) {
        m_coro = other.m_coro;
        other.m_coro = nullptr;
    }

    ~await_return_object() {
        if (m_coro) {
            m_coro.destroy();
        }
    }


    // promise type must have either return_void or return_value member but not both
    // not even if one is SFINAEd out - you cannot have both names present, per Lewis Baker
    template<typename U>
    struct promise_base {
        void return_value(U const&) const noexcept {
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

        await_return_object get_return_object() {
            return await_return_object(*this);
        }

        void unhandled_exception() {}  // do nothing :)

    };


private:
    std::experimental::coroutine_handle<promise_type> m_coro;

};

template<>
template<>
struct await_return_object<void>::promise_base<void> {
    void return_void() const noexcept {
    }
};

#endif // CO_AWAITER_HPP
