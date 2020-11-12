// A simple Awaitable type for experiments
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

#ifndef MY_AWAITABLE_HPP
#define MY_AWAITABLE_HPP

#include <experimental/coroutine>
#include <type_traits>

template<typename F,
         typename ReturnType = typename std::remove_cv_t<std::invoke_result_t<F>>>
struct my_awaitable {
    // construct with a nullary function that does something and returns a value
    my_awaitable(F work_fn) : work_(work_fn) {}

    struct awaiter {
        awaiter(my_awaitable* awaitable) : awaitable_(awaitable) {}

        bool await_ready() const noexcept { return false; }  // pretend to not be ready

        ReturnType await_resume()      noexcept { return awaitable_->work_(); }

        template<typename P>
        void await_suspend(std::experimental::coroutine_handle<P> coro) noexcept {
            // Per Lewis Baker, the variant of await_suspend() that returns bool means:
            // true: execution returns to the caller of *our* resume()
            // false: execution continues immediately on the calling thread

            // the void-returning version which we are using is the same as returning "true"

            // decide we are ready after all, so resume caller of co_await
            coro.resume();

            // we could also leave off the call to resume() and return false from a bool version
            // of this function. Or we can return true from await_ready(), and this won't run
            // at all.
        }

        my_awaitable* awaitable_;   // remember parent

    };
    awaiter operator co_await () { return awaiter{this}; }

private:
    F work_;
};

// type deduction helper
template<typename F>
my_awaitable<F>
make_my_awaitable(F fn) {
    return my_awaitable{fn};
}

#endif // MY_AWAITABLE_HPP
