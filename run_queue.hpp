// The world's lamest event loop/run queue
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

#ifndef RUN_QUEUE_HPP
#define RUN_QUEUE_HPP

#include <queue>
#include <functional>

// solely for the purpose of queueing up work to run later, as a way to test callbacks etc.
// This run queue does as little as possible:
// all you can do is queue tasks; a single thread runs them, and when there are none remaining
// it exits

struct run_queue {
    using task = std::function<void(run_queue*)>;

    template<typename F>
    void add_task(F f) {
        tasks_.push(std::move(f));
    }

    void run();

private:
    std::queue<task> tasks_;
};

#endif // RUN_QUEUE_HPP
