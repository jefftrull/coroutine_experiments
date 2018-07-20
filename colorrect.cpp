// Implementation of a simple Qt colored rectangle widget, for experiments
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

#include <QMouseEvent>
#include <QPainter>

#include "colorrect.h"

ColorRect::ColorRect(QWidget *parent) :
    QWidget{parent},
    colorList{{"#111111", "#113311",
                "#111133", "#331111",
                "#333311", "#331133",
                "#661111", "#116611",
                "#111166", "#663311",
                "#661133", "#336611",
                "#331166", "#113366"}},
    curColor{0}
{}

void
ColorRect::setColor(std::string const& col)
{
    setStyleSheet(("background-color:" + col).c_str());
}

void
ColorRect::changeColor()
{
    curColor++;
    if (curColor >= colorList.size())
    {
        curColor = 0;
    }
    setColor(colorList[curColor]);
}

void
ColorRect::mousePressEvent(QMouseEvent *e)
{
    emit click(e->windowPos());
}

void
ColorRect::paintEvent(QPaintEvent *)
{
    if (line) {
        QPainter painter(this);
        painter.setPen(QPen{QColor{"yellow"}});
        painter.drawLine(*line);
    }
}

void
ColorRect::setLine(QPointF p1, QPointF p2)
{
    line = QLineF{p1, p2};
}
