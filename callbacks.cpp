// Doing some simple work with callbacks on a run queue
// Copyright (c) 2018 Jeff Trull <edaskel@att.net>

#include <iostream>
#include "run_queue.hpp"

int main() {
    run_queue work;
    work.add_task([](run_queue* tasks) {
            std::cout << "first task\n";
            // first task adds a second one as a "callback"
            tasks->add_task([](run_queue* tasks) {
                    std::cout << "second task\n";
                });
        });
    work.run();
}
