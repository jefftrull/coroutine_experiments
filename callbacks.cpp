// Doing some simple work with callbacks on a run queue
// Copyright (c) 2018 Jeff Trull <edaskel@att.net>

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
    // which I hope to ultimately do with coroutines

    work.add_task([](run_queue* tasks) {
            // future coroutine
            int a = 2;
            int b = 3;
            int c = 4;
            // queue long-running task
            tasks->add_task([=](run_queue* tasks) {
                    multiply(a, b,
                             [c](int product) {
                                 int result = product + c;
                                 std::cout << "result: " << result << "\n";
                             });
                });
        });
    work.run();
}
