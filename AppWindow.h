#ifndef __meshinspect__AppWindow__
#define __meshinspect__AppWindow__

#include <QMainWindow>
#include <QAction>
#include <QScrollArea>
#include "NodeList.h"
#include "MeshView.h"

class AppWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    AppWindow();
    void open(const QString file);
    
public slots:
    void open();
    void save();
    void saveAs();
    void getProjection();
    void saveDepth();
    void saveMaxArea();
    void saveMaxLength();
    void saveMaxDH();
    void saveMaxDT();
    void saveMinArea();
    void saveMinLength();
    void saveMinDH();
    void saveMinDT();
    void zoomIn();
    void zoomOut();
    void zoomNormal();
    void zoomWindow(bool fixed);
    
private:
    NodeList * appData;
    QAction * polarStats;
    QScrollArea * meshScrollArea;
    MeshView * meshPlot;
    qreal meshScale;
    
    QAction * zoomOutAct;
    QAction * zoomInAct;
    QAction * zoomNormalAct;
    
    void setupMenu();
    void setupAppTabs();
};

#endif /* defined(__meshinspect__AppWindow__) */
