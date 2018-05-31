// Doing some simple work with callbacks on a run queue
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
#include "run_queue.hpp"

// a representative expensive/high latency calculation
// that delivers results with a callback
template<typename Callback>
void multiply(int a, int b, Callback cb) {
    int result = a * b;
    cb(result);
}

int main() {
    run_queue work;

    // use the work queue to calculate 2*3+4 in a really dumb way
    // we launch a task to calculate 2*3, with a callback that adds 4
    // this models a long-running task (the multiply) with a finishing callback
    // the coroutine version is in cb_as_coro.cpp

    work.add_task([](run_queue* tasks) {
            // future coroutine
            int a = 2;
            int b = 3;
            int c = 4;
            // queue long-running task
            tasks->add_task([=](run_queue*) {
                    multiply(a, b,
                             [c](int product) {
                                 int result = product + c;
                                 std::cout << "result: " << result << "\n";
                             });
                });
        });
    work.run();
}
