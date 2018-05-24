// My callback example using co_await AND Asio's io_service as a run queue

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

// compute a*b+c on an Asio run queue with the multiply awaited
// In a way this is a combination of the original callback example (with
// lame hand-crafted run queue) and the co_awaited multiply (handled
// synchronously) in cb_as_coro.cpp.

#include <iostream>

#include <experimental/coroutine>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

using boost::asio::experimental::co_spawn;
namespace this_coro = boost::asio::experimental::this_coro;
using boost::asio::io_context;

template <typename T>
  using awaitable = boost::asio::experimental::awaitable<
    T, boost::asio::io_context::executor_type>;

awaitable<int> multiply(int x, int y) {
    // arbitrary async operation so we suspend and resume from the run queue
    auto token = co_await boost::asio::experimental::this_coro::token();
    boost::asio::steady_timer t(token.get_executor().context(),
                                boost::asio::chrono::milliseconds(50));
    co_await t.async_wait(token);  // suspend and run something else
    co_return x * y;
}

awaitable<int> muladd() {
    int a = 2;
    int b = 3;
    int c = 4;
    int product = co_await multiply(a, b);           // runs directly, not through io_context
    int result = product + c;
    co_return result;
}

int main() {
    io_context io;

    // our main work
    co_spawn(io,                                     // where to run
             [](){ return muladd(); },               // what to do
             [](std::exception_ptr e, int result) {  // completion handler
                 std::cout << "result: " << result << "\n";
             });

    // something to do to show that we return to the run queue when we co_await the timer
    // inserted after the above, so lower priority, but gets finished first.
    boost::asio::post(io, []() { std::cout << "intermediate run queue task\n"; });

    io.run();

}
