#include <QWidget>
#include <QPolygon>
#include <QPoint>
#include <QLine>
#include <QPaintEvent>
#include <QRect>
#include <QPainter>
#include <QPicture>
#include "MeshView.h"
#include "NodeList.h"
#include <stdio.h>
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

QVector<QBrush> MeshView::depthMesh(nodelist * nlist, triangle ** ele, unsigned int nele)
{
    double depths[nele];
    double min, max;
    
    depths[0] = get_depth(nlist, ele[0]);
    min = max = depths[0];
    for (unsigned int i = 0; i < nele; i++) {
        depths[i] = get_depth(nlist, ele[i]);
        if (depths[i] > max)
            max = depths[i];
        else if (depths[i] < min)
            min = depths[i];
    }
    
    QColor col("mediumslateblue");
    QVector<QBrush> vect(nele);
    for (unsigned int i = 0; i < nele; i++) {
        int a;
        if (min != max)
            a = (int)(255.0*(depths[i] - min)/(max - min));
        else
            a = 0;
        col.setAlpha(a);
        vect[i] = QBrush(col);
    }
    return vect;
}

void MeshView::updateData()
{
    nodelist * nlist = model->data();
    if (nlist == NULL) return;
    nodePoly.resize(nlist->numNodes);
    boundNodes.clear();
    islandNodes.clear();
    
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * n = nlist->nodes[i];
        nodePoly[n->number] = QPoint((int)(n->x*1000), (int)(n->y*1000));
    }
    
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * n = nlist->nodes[i];
        if (n->type == NOD_EXTERIOR) boundNodes.append(nodePoly[n->number]);
        else if (n->type == NOD_ISLAND) islandNodes.append(nodePoly[n->number]);
    }
    
    unsigned int count;
    triangle ** elems = get_elements(nlist, &count);
    QVector<QBrush> depthc = depthMesh(nlist, elems, count);
    QPainter bat;
    bat.begin(&depthPic);
    bat.setPen(QPen(QColor("darkslateblue"), 0, Qt::SolidLine, Qt::RoundCap));
    for (unsigned int i = 0; i < count; i++) {
        QPainterPath tri;
        triangle * t = elems[i];
        QPoint n1 = nodePoly[t->n1];
        QPoint n2 = nodePoly[t->n2];
        QPoint n3 = nodePoly[t->n3];
        tri.moveTo(n1);
        tri.lineTo(n2);
        tri.lineTo(n3);
        tri.lineTo(n1);
        
        bat.setBrush(depthc[i]);
        bat.drawPath(tri);
    }
    bat.end();
    
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
    painter.drawPicture(0, 0, depthPic);
    painter.setPen(QPen(Qt::green, 5.0/scaleVal, Qt::SolidLine, Qt::RoundCap));
    painter.drawPoints(islandNodes);
    painter.setPen(QPen(Qt::blue, 5.0/scaleVal, Qt::SolidLine, Qt::RoundCap));
    painter.drawPoints(boundNodes);
}

