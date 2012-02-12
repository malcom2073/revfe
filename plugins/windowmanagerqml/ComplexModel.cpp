#include "ComplexModel.h"
#include <QDebug>
ComplexModel::ComplexModel(QObject *parent) : QAbstractItemModel(parent)
{

}

void ComplexModel::setRoles(QStringList roleslist)
{
	QHash<int, QByteArray> roles;
	for (int i=0;i<roleslist.size();i++)
	{
		roles[i] = roleslist[i].toAscii();
	}
	setRoleNames(roles);
}

int ComplexModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return m_dataList.size();
}
int ComplexModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	if (m_dataList.size() > 0)
	{
		return m_dataList[0].size();
	}
	else
	{
		return 0;
	}
}
QModelIndex ComplexModel::index(int row, int column, const QModelIndex &parent) const
{

	return createIndex(row,column,0);
}
QModelIndex ComplexModel::parent(const QModelIndex &child) const
{
	return QModelIndex();
}
void ComplexModel::setRowCount(int count)
{
	if (m_dataList.size() == count)
	{
		return;
	}
	if (m_dataList.size() < count)
	{
		beginInsertRows(QModelIndex(),m_dataList.size()-1,count-1);
		while (m_dataList.size() < count)
		{
			m_dataList.append(QList<QString>());
		}
		endInsertRows();
	}
	if (m_dataList.size() > count)
	{
		beginRemoveRows(QModelIndex(),m_dataList.size()-1,count-1);
		while (m_dataList.size() > count)
		{
			m_dataList.removeLast();
		}
		endRemoveRows();
	}
}

void ComplexModel::setColumnCount(int count)
{
	beginResetModel();
	for (int i=0;i<m_dataList.size();i++)
	{
		while (m_dataList[i].size() < count)
		{
			m_dataList[i].append(QString());
		}
		while (m_dataList[i].size() > count)
		{
			m_dataList[i].removeLast();
		}
	}
	reset();
	//endResetModel();
}
void ComplexModel::addRow(QList<QString> row)
{
	beginInsertRows(QModelIndex(),rowCount(QModelIndex()),rowCount(QModelIndex()));
	m_dataList.append(row);
	endInsertRows();
}

void ComplexModel::setData(QList<QList<QString> > data)
{
	beginResetModel();
	m_dataList = data;
	reset();
}

QVariant ComplexModel::data(const QModelIndex &index, int role) const
{
	if (m_dataList.size() > index.row())
	{
		if (m_dataList[index.row()].size() > role)
		{
			//qDebug() << "Count:" << m_dataList.size() << m_dataList[index.row()].size() << role;
			//qDebug() << index.row() << m_dataList[index.row()][role];
			return QVariant(m_dataList[index.row()][role]);
		}
	}
	return QVariant();
}



