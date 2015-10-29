#ifndef __meshinspect__AppWindow__
#define __meshinspect__AppWindow__

#include <QMainWindow>
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
    
private:
    NodeList * appData;
    
    void setupMenu();
    void setupAppTabs();
};

#endif /* defined(__meshinspect__AppWindow__) */
