#include <QWidget>
#include <QPolygon>
#include <QPoint>
#include <QLine>
#include <QPaintEvent>
#include <QRect>
#include <QPainter>
#include "MeshView.h"
#include "NodeList.h"

extern "C" {
    #include "nodeUtil.h"
}

MeshView::MeshView(QWidget * parent) : QWidget(parent)
{
    model = NULL;
}

QSize MeshView::sizeHint() const
{
    return QSize(450, 450);
}

void MeshView::setModel(NodeList * nodes)
{
    model = nodes;
    connect(model, SIGNAL (listUpdated()), this, SLOT (updateData()));
    updateData();
}

void MeshView::updateData()
{
    nodelist * nlist = model->data();
    if (nlist == NULL) return;
    
    nodePoly.resize(nlist->numNodes);
    lineVect.clear();
    boundNodes.clear();
    islandNodes.clear();
    
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * n = nlist->nodes[i];
        nodePoly[n->number] = QPoint((int)(n->x*1000), (int)(n->y*1000));
    }
    
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * n = nlist->nodes[i];
        for (short j = 0; j < n->neighbourCount; j++) {
            unsigned int k = n->neighbours[j];
            if (k >= n->number) continue;
            QPoint n1 = nodePoly[n->number];
            QPoint n2 = nodePoly[k];
            lineVect.append(QLine(n1, n2));
        }
        if (n->type == NOD_EXTERIOR) boundNodes.append(nodePoly[n->number]);
        else if (n->type == NOD_ISLAND) islandNodes.append(nodePoly[n->number]);
    }
    
    meshRect = nodePoly.boundingRect();
    int meshSize = qMax(size().width(), size().height());
    int margin = 10.0*(qreal)qMax(meshRect.width(), meshRect.height())/(qreal)meshSize;
    meshRect += QMargins(margin, margin, margin, margin);
    
    update();
}

void MeshView::paintEvent(QPaintEvent * event)
{
    if (nodePoly.isEmpty()) return;
    
    int meshSize = qMin(size().width(), size().height());
    qreal scaleVal = meshSize/(qreal)qMax(meshRect.width(), meshRect.height());
    qreal tx = (qreal)size().width()/2.0 - (qreal)meshRect.center().x()*scaleVal;
    qreal ty = (qreal)size().height()/2.0 + (qreal)meshRect.center().y()*scaleVal;
    qreal sx = scaleVal;
    qreal sy = -1.0*scaleVal;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(tx, ty);
    painter.scale(sx, sy);
    painter.setPen(QPen(Qt::black, 0, Qt::SolidLine, Qt::RoundCap));
    painter.drawLines(lineVect);
    painter.setPen(QPen(Qt::green, 5.0/scaleVal, Qt::SolidLine, Qt::RoundCap));
    painter.drawPoints(islandNodes);
    painter.setPen(QPen(Qt::blue, 5.0/scaleVal, Qt::SolidLine, Qt::RoundCap));
    painter.drawPoints(boundNodes);
}

