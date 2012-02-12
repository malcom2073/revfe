#ifndef STANDARDMODEL_H
#define STANDARDMODEL_H

#include <QStandardItemModel>

class StandardModel : public QStandardItemModel
{
public:
    StandardModel(const QHash<int, QByteArray> &roleNames);
    //static QVariantMap getModelData(const QAbstractItemModel *model, int row);
};

#endif // STANDARDMODEL_H
