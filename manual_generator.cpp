// My attempt at creating a simple generator that does *not* store its coroutine state using
// coroutine_handle etc. but instead has its own mechanism

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

#include <iostream>
#include <experimental/coroutine>

namespace detail
{
// a piece of state accessible to all generators
static int counter = 0;
}

// basic generator-type coroutine (no co_await)

// the return object
// this is the "return value" of our coroutine, but it never gets returned
// instead what happens is the coroutine is created and we are handed an instance
// of this type to use to interact with the coroutine using whatever methods we
// have defined here
struct my_return {

    // the "promise type" has to be defined or declared here - it is a requirement
    // of the coroutine machinery and must have certain specific methods
    struct promise_type {

        promise_type() : m_current_value(-1) {}

        // coroutine promise requirements:

        auto initial_suspend() const noexcept {
            // we don't need to return to the creator of the coroutine prior to
            // doing work so:
            return std::experimental::suspend_never(); // produce at least one value
        }

        auto final_suspend() const noexcept {
            // suspend "always" if someone else destroys the coroutine
            // (as we do in the my_return destructor)
            // choose "never" if it runs off the end and destroys itself
            // our coroutine has an infinite loop so we will never get here but
            // for the sake of form:
            return std::experimental::suspend_always();
        }

        void return_void() const noexcept {}

        my_return get_return_object() {
            return my_return(*this);
        }

        auto yield_value(int value) {
            m_current_value = value;
            return std::experimental::suspend_always();
        }

        void unhandled_exception() {}  // do nothing :)

        int m_current_value;

    };

    // end promise requirements

    // my API

    my_return(promise_type & p) : m_coro(std::experimental::coroutine_handle<promise_type>::from_promise(p)) {}

    my_return(my_return const&) = delete;

    my_return(my_return && other) : m_coro(other.m_coro) {
        other.m_coro = nullptr;
    }

    int value() const {
        return m_coro.promise().m_current_value;
    }

    void advance() {
        // advance coroutine to next co_yield
        m_coro.resume();
    }

    ~my_return() {
        if (m_coro)
            m_coro.destroy();
    }

private:
    std::experimental::coroutine_handle<promise_type> m_coro;
};

my_return my_coro() {
    while (1) {
        co_yield detail::counter++;
    }
}

int main() {
    auto p = my_coro();             // immediate suspension with promise returned
    std::cout << p.value() << "\n";
    p.advance();
    std::cout << p.value() << "\n";
    p.advance();
    std::cout << p.value() << "\n";
}
