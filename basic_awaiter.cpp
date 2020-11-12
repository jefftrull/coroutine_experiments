// Incorporating an Awaitable

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
#include "co_awaiter.hpp"
#include "my_awaitable.hpp"

namespace detail
{
// a piece of state accessible to all generators
static int counter = 0;
}

// now we need a coroutine to perform co_await on my awaitable
// co_await must be inside a coroutine, and main cannot be a coroutine, so...


await_return_object<> try_awaiting() {
    auto awaitable = make_my_awaitable([](){ return detail::counter++; });
    // count to five
    // this causes a stack overflow for large values; see my_awaitable.hpp
    // for an explanation
    for (auto i = co_await awaitable; i != 5;i = co_await awaitable) {
        std::cout << "not there yet: " << i << "\n";
    }
    std::cout << "done!\n";
}

int main() {
    auto coro = try_awaiting();
}
