// The world's lamest event loop/run queue implementation
// Copyright (c) 2018 Jeff Trull <edaskel@att.net>

#include "run_queue.hpp"

void run_queue::run() {
    while (!tasks_.empty()) {
        tasks_.front()(this);
        tasks_.pop();
    }
}
