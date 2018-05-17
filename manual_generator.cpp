// Copyright Jeff Trull <edaskel@att.net>
// My attempt at creating a simple generator that does *not* store its coroutine state using
// coroutine_handle etc. but instead has its own mechanism

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
            return std::experimental::suspend_always(); // ?? not sure
        }

        auto final_suspend() const noexcept {
            return std::experimental::suspend_always(); // ?? not sure
        }

        auto return_void() const noexcept {
            return std::experimental::suspend_always(); // ?? not sure
        }

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

private:
    std::experimental::coroutine_handle<promise_type> m_coro;
};

my_return my_coro() {
    while (1) {
        co_yield detail::counter++;
    }
}

int main() {
    auto p = std::move(my_coro());   // immediate suspension with promise returned
    std::cout << p.value() << "\n";
    p.advance();
    std::cout << p.value() << "\n";
    p.advance();
    std::cout << p.value() << "\n";
}
