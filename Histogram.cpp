#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <math.h>
#include "Histogram.h"

Histogram::Histogram(const QString key, QWidget * parent) : QWidget(parent)
{
    page = new QString(key);
    model = NULL;
    logScale = false;
}

void Histogram::setModel(NodeList * nodes)
{
    model = nodes;
    updateData();
}

void Histogram::setScale(bool isLog)
{
    logScale = isLog;
    updateData();
}

void Histogram::updateData()
{
    if (model == NULL) return;
    
    int * data = model->hist(*page);
    if (data == NULL) return;
    
    rects.clear();
    for (int i = 0; i < HIST_WIDTH; i++) {
        if (logScale)
            rects.append(QRectF(i + 1, 0, 1, log10((float)data[i] + 0.01)));
        else
            rects.append(QRect(i + 1, 0, 1, data[i]));
    }
    update();
}

void Histogram::paintEvent(QPaintEvent * event)
{
    if (rects.isEmpty()) return;
    
    const QRect viewRect = event->rect();
    QPainter painter(this);
    float max = 1;
    for (int i = 0; i < rects.count(); i++) if (rects[i].height() > max) max = rects[i].height();
    
    painter.translate(0, viewRect.height());
    painter.scale((float)viewRect.width()/(float)rects.count(), -1.0*(float)viewRect.height()/max);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::blue);
    painter.drawRects(rects);
}

QSize Histogram::sizeHint() const
{
    return QSize(HIST_WIDTH, HIST_HEIGHT);
}

