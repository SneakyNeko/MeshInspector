#ifndef __meshinspect__MeshView__
#define __meshinspect__MeshView__

#include <QWidget>
#include <QPolygon>
#include <QVector>
#include <QLine>
#include "NodeList.h"

class MeshView : public QWidget
{
    Q_OBJECT
    
public:
    MeshView(QWidget * parent = 0);
    
    QSize sizeHint() const Q_DECL_OVERRIDE;
    void setModel(NodeList * nodes);
    void setFitToWindow(bool fit);
    bool fitToWindow();
    
public slots:
    void updateData();
    
protected:
    void paintEvent(QPaintEvent * event) Q_DECL_OVERRIDE;
    
private:
    QPolygon nodePoly;
    QPolygon boundNodes;
    QPolygon islandNodes;
    QVector<QLine> lineVect;
    NodeList * model;
    QRect meshRect;
};

#endif /* defined(__meshinspect__MeshView__) */
