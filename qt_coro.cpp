// example use of the Coroutines TS with Qt signals and slots
// This is qt_basic.cpp ported to use coroutines
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

#include <QApplication>
#include <QTimer>

#include "colorrect.h"
#include "qtcoro.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // a really simple widget
    ColorRect cr;
    cr.setWindowTitle("Color Cycler");
    cr.show();

    // change widget color every 500ms
    QTimer * changeTimer = new QTimer(&app);
    auto ro = [&]() -> qtcoro::return_object<> {
        while (true) {
            co_await qtcoro::make_awaitable_signal(changeTimer, &QTimer::timeout);
            cr.changeColor();
        }
    }();

    changeTimer->start(500);

    // draw lines from clicks
    auto ptclick_ro = [&]() -> qtcoro::return_object<> {
        while (true) {
            QPointF first_point = co_await qtcoro::make_awaitable_signal(&cr, &ColorRect::click);
            QPointF second_point = co_await qtcoro::make_awaitable_signal(&cr, &ColorRect::click);
            cr.setLine(first_point, second_point);
        }
    }();

    // listen for line creation (tests the tuple code)
    auto line_ro = [&]() -> qtcoro::return_object<> {
        while (true) {
            auto [p1, p2] = co_await qtcoro::make_awaitable_signal(&cr, &ColorRect::lineCreated);
            std::cout << "we drew a line from (";
            std::cout << p1.x() << ", " << p1.y() << ") to (";
            std::cout << p2.x() << ", " << p2.y() << ")\n";
        }
    }();

    return app.exec();
}

