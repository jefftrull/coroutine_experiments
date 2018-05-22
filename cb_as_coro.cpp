// My callback example refactored into coroutines
/*
Copyright (c) 2018 Jeff Trull <edaskel@att.net

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

// compute a*b+c in a coroutine in two stages, awaiting the multiply result

// This is the coroutine equivalent of my multiply callback code in callbacks.cpp

#include <iostream>
#include "my_awaitable.hpp"
#include "co_awaiter.hpp"

await_return_object<> muladd() {
    int a = 2;
    int b = 3;
    int c = 4;
    // wait for our "long-running" task (previously handled by a callback)
    int product = co_await make_my_awaitable([a,b]() { return a*b; });
    int result = product + c;
    std::cout << "result: " << result << "\n";
}

int main() {
    auto coro = muladd();
}

