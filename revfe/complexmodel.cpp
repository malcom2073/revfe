#include "complexmodel.h"
#include <QDebug>
#include <QMutexLocker>
ComplexModel::ComplexModel(QObject *parent) : QAbstractItemModel(parent)
{
	qRegisterMetaType<QModelIndex>("QModelIndex");
	accessMutex = new QMutex();
}

void ComplexModel::setRoles(QStringList roleslist)
{
	accessMutex->lock();
	QHash<int, QByteArray> roles;
	for (int i=0;i<roleslist.size();i++)
	{
		roles[i] = roleslist[i].toAscii();
	}
	setRoleNames(roles);
	accessMutex->unlock();
}

int ComplexModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	//QMutexLocker myMutex(accessMutex);
	return m_dataList.size();
}
void ComplexModel::clearContents()
{
	accessMutex->lock();
	beginResetModel();
	m_dataList.clear();
	//m_dataList.append(QList<QString>() << "Testing");
	endResetModel();
	accessMutex->unlock();
}

int ComplexModel::columnCount(const QModelIndex &parent) const
{
	QMutexLocker myMutex(accessMutex);
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
	if (row < 0 || column < 0 || row >= m_dataList.size())
	{
		return QModelIndex();
	}
	if (m_dataList.size() == 0)
	{
		return QModelIndex();
	}
	else
	{
		if (column >= m_dataList[0].size())
		{
			return QModelIndex();
		}
	}
	return createIndex(row,column,0);
}
/*QModelIndex ComplexModel::parent(const QModelIndex &child) const
{
	return index(child.row(),0,QModelIndex());
}*/
void ComplexModel::setRowCount(int count)
{
	QMutexLocker myMutex(accessMutex);
	if (m_dataList.size() == count)
	{
		return;
	}
	if (m_dataList.size() < count)
	{
		beginInsertRows(QModelIndex(),m_dataList.size(),count);
		while (m_dataList.size() < count)
		{
			m_dataList.append(QList<QString>());
		}
		endInsertRows();
	}
	if (m_dataList.size() > count)
	{
		beginRemoveRows(QModelIndex(),m_dataList.size(),count);
		while (m_dataList.size() > count)
		{
			m_dataList.removeLast();
		}
		endRemoveRows();
	}
}

void ComplexModel::setColumnCount(int count)
{
	accessMutex->lock();
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
	endResetModel();
	accessMutex->unlock();
	//endResetModel();
}
void ComplexModel::addRow(QList<QString> row)
{
	accessMutex->lock();
	//beginInsertRows(QModelIndex(),m_dataList.count()-1,m_dataList.count()-2);

	//This is a stupid fix, that is required because beginInsertRows dosen't operate properly
	//with a no size list. Probably a bug, unknown as the Qt folks aren't especially helpful
	//on irc.
	//if (m_dataList.count() == 0)
	//{
		//beginInsertRows(QModelIndex(),m_dataList.count()-1,m_dataList.count()-2);
	//}
	//else
	//{

		beginInsertRows(createIndex(m_dataList.count(),0,0),m_dataList.count(),m_dataList.count());
	//}
	m_dataList.append(row);
	endInsertRows();
	accessMutex->unlock();
}

void ComplexModel::setData(QList<QList<QString> > data)
{
	accessMutex->lock();
	beginResetModel();
	m_dataList = data;
	endResetModel();
	accessMutex->unlock();
}

QVariant ComplexModel::data(const QModelIndex &index, int role) const
{
	QMutexLocker myMutex(accessMutex);
	if (!index.isValid())
	{
		return QVariant();
	}
	if (m_dataList.size() > index.row())
	{
		if (m_dataList[index.row()].size() > role)
		{
			//qDebug() << "Count:" << m_dataList.size() << m_dataList[index.row()].size() << role;
			//qDebug() << index.row() << m_dataList[index.row()][role];
			return m_dataList[index.row()][role];
		}
	}
	return QVariant();
}



