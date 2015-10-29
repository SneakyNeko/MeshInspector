#ifndef __meshinspect__StatsTable__
#define __meshinspect__StatsTable__

#include <QAbstractTableModel>
#include "NodeList.h"

class StatsTable : public QAbstractTableModel
{
    Q_OBJECT
    
public:
    StatsTable(const QString & page, NodeList * data, QObject * parent);
    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    
private:
    NodeList * dataModel;
    QString * pageKey;
    QList<QString> headers;
};

#endif /* defined(__meshinspect__StatsTable__) */
