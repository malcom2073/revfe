#ifndef COMPLEXMODEL_H
#define COMPLEXMODEL_H
#include <QAbstractListModel>
#include <QStringList>
#include <QMutex>
class ComplexModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	ComplexModel(QObject *parent=0);
	void setRoles(QStringList roles);
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent) const;
	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	//QModelIndex parent(const QModelIndex &child) const;
	QVariant data(const QModelIndex &index, int role) const;
	void setRowCount(int count);
	void setColumnCount(int count);
	void setData(QList<QList<QString> > data);
	void addRow(QList<QString> row);
	void clearContents();
private:
	QList<QList<QString> > m_dataList;
	QMutex *accessMutex;
};

#endif // COMPLEXMODEL_H
