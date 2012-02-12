#ifndef COMPLEXMODEL_H
#define COMPLEXMODEL_H
#include <QAbstractListModel>
#include <QStringList>
class ComplexModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	ComplexModel(QObject *parent=0);
	void setRoles(QStringList roles);
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &child) const;
	QVariant data(const QModelIndex &index, int role) const;
	void setRowCount(int count);
	void setColumnCount(int count);
	void setData(QList<QList<QString> > data);
	void addRow(QList<QString> row);
private:
	QList<QList<QString> > m_dataList;
};

#endif // COMPLEXMODEL_H
