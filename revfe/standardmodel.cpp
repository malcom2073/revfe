#include "standardmodel.h"

StandardModel::StandardModel(const QHash<int, QByteArray> &roleNames) : QStandardItemModel()
{
	setRoleNames(roleNames);
}
/*
QVariantMap StandardModel::getModelData(const QAbstractItemModel* model, int row)
{
	QHash<int,QByteArray> rolenames = model->roleNames();
	QHashIterator<int, QByteArray> i(rolenames);
	QVariantMap retval;
	while (i.hasNext())
	{
		i.next();
		QModelIndex index = model->index(row, 0);
		QVariant data = index.data(i.key());
		retval[i.value()] = data;
	}
	return retval;
}


*/
