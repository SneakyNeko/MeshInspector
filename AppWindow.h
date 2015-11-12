#ifndef __meshinspect__AppWindow__
#define __meshinspect__AppWindow__

#include <QMainWindow>
#include <QAction>
#include "NodeList.h"

class AppWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    AppWindow();
    AppWindow(const QString & file);
    
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
    
private:
    NodeList * appData;
    QAction * polarStats;
    
    void setupMenu();
    void setupAppTabs();
};

#endif /* defined(__meshinspect__AppWindow__) */
