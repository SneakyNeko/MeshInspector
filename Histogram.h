#ifndef __meshinspect__Histogram__
#define __meshinspect__Histogram__

#include <QWidget>
#include "NodeList.h"

#define HIST_WIDTH 500
#define HIST_HEIGHT 400

class Histogram : public QWidget
{
    Q_OBJECT
    
public:
    Histogram(const QString key, QWidget * parent = 0);
    
    QSize sizeHint() const Q_DECL_OVERRIDE;
    void setModel(NodeList * nodes);
    
public slots:
    void updateData();
    void setScale(bool isLog);
    
protected:
    void paintEvent(QPaintEvent * event) Q_DECL_OVERRIDE;
  
private:
    QVector<QRectF> rects;
    QString * page;
    NodeList * model;
    bool logScale;
};

#endif /* defined(__meshinspect__Histogram__) */
