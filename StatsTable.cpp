#include <QAbstractTableModel>
#include "StatsTable.h"
#include "NodeList.h"

StatsTable::StatsTable(const QString & page, NodeList * data, QObject * parent) : QAbstractTableModel(parent)
{
    dataModel = data;
    pageKey = new QString(page);
    headers << "Max" << "Min" << "Mean" << "Std. Deviation";
}

int StatsTable::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) return 0;
    return 1;
}

int StatsTable::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid()) return 0;
    return headers.count();
}

QVariant StatsTable::data(const QModelIndex & index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) return QVariant();
    
    double * dat = dataModel->statistics(*pageKey);
    return dat[index.column()];
}

QVariant StatsTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    
    if (orientation == Qt::Horizontal) return headers[section];
    else return QVariant();
}

