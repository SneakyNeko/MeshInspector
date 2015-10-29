#ifndef __meshinspect__NodeList__
#define __meshinspect__NodeList__

#include <QObject>
#include <QMap>

extern "C" {
    #include "nodeUtil.h"
}

class NodeList : public QObject
{
    Q_OBJECT
    
public:
    NodeList();
    ~NodeList();
    
    int * hist(const QString key);
    double * statistics(const QString key);
    nodelist * data();
    void loadFile(const QString file);
    void saveFile(const QString file);
    void setProjection(const QString proj);
    QString projection();
    
    QMap<QString, int> pageKeys;
    
    static const QString fileFilter;
    
signals:
    void listUpdated();
    void updateScales(bool isLog);
    
public slots:
    void setUnits(bool isPolar);
    
private:
    nodelist * nodes;
    int ** histData;
    double ** stats;
    QString * proj;
};

#endif /* defined(__meshinspect__NodeList__) */
