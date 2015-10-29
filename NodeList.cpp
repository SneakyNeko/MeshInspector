#include <QObject>
#include <QMap>
#include <cstdlib>
#include "NodeList.h"
#include "Histogram.h"

extern "C" {
    #include "nodeUtil.h"
}

const QString NodeList::fileFilter = "Neighbor File (*.nei);;Split Files(*.nod *.ele *.bat);;All Files (*.*)";

NodeList::NodeList()
{
    histData = (int **)malloc(sizeof(int *)*PAGES);
    stats = (double **)malloc(sizeof(double *)*PAGES);
    for (int i = 0; i < PAGES; i++) {
        histData[i] = (int *)malloc(sizeof(int)*HIST_WIDTH);
        stats[i] = (double *)malloc(sizeof(double)*4);
        for (int j = 0; j < HIST_WIDTH; j++) histData[i][j] = 0;
        for (int j = 0; j < 4; j++) stats[i][j] = 0;
    }
    proj  = NULL;
    nodes = NULL;
    pageKeys["Dh/h"]  = 0;
    pageKeys["Area"]  = 1;
    pageKeys["Depth"] = 2;
    pageKeys["Side"]  = 3;
    pageKeys["dt"]    = 4;
}

NodeList::~NodeList()
{
    if (histData != NULL) for (int i = 0; i < PAGES; i++) free(histData[i]);
    if (stats != NULL) for (int i = 0; i < PAGES; i++) free(stats[i]);
    free(histData);
    free(stats);
    freeList(nodes);
    pageKeys.clear();
}

int * NodeList::hist(const QString key)
{
    if (!pageKeys.contains(key)) return NULL;
    
    return histData[pageKeys[key]];
}

double * NodeList::statistics(const QString key)
{
    if (!pageKeys.contains(key)) return NULL;
    
    return stats[pageKeys[key]];
}

nodelist * NodeList::data()
{
    return nodes;
}

void NodeList::loadFile(const QString file)
{
    if (file.isEmpty()) return;
    freeList(nodes);
    
    if (file.endsWith(".nei", Qt::CaseInsensitive))
        nodes = read_nei(qPrintable(file));
    else if ( file.endsWith(".ele", Qt::CaseInsensitive)
           || file.endsWith(".bat", Qt::CaseInsensitive)
           || file.endsWith(".nod", Qt::CaseInsensitive) ) {
        QString fileStub(file);
        fileStub.chop(4);
        nodes = read_neb(qPrintable(fileStub));
    } else return;
    
    reorder_nodes(nodes);
    fix_boundaries(nodes);
    boundary_stats(nodes, histData, stats, 0);
    proj = new QString(nodes->proj);
    
    emit listUpdated();
}

void NodeList::saveFile(const QString file)
{
    if (file.isEmpty())
        return;
    else if ( file.endsWith(".nei", Qt::CaseInsensitive) )
        write_nei(nodes, qPrintable(file));
    else if ( file.endsWith(".ele", Qt::CaseInsensitive)
           || file.endsWith(".bat", Qt::CaseInsensitive)
           || file.endsWith(".nod", Qt::CaseInsensitive) ) {
        QString fileStub(file);
        fileStub.chop(4);
        write_neb(nodes, qPrintable(fileStub));
    }
}

void NodeList::setProjection(const QString proj)
{
    this->proj = new QString(proj);
    nodes->proj = (char *)malloc(proj.length() + 1);
    strcpy(nodes->proj, qPrintable(proj));
    project(nodes);
    
    emit listUpdated();
}

QString NodeList::projection()
{
    if (proj == NULL) return QString();
    return *proj;
}

void NodeList::setUnits(bool isPolar)
{
    if (isPolar)
        boundary_stats(nodes, histData, stats, 1);
    else
        boundary_stats(nodes, histData, stats, 0);
    
    emit listUpdated();
}

