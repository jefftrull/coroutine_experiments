// The world's lamest event loop/run queue
// Copyright (c) 2018 Jeff Trull <edaskel@att.net>

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
