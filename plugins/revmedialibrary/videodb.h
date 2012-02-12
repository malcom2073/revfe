#ifndef VIDEODB_H
#define VIDEODB_H
#include <QObject>
#include <QSqlDatabase>

class VideoDB : public QObject
{
	Q_OBJECT
public:
	VideoDB(QObject *parent=0);
	QList<QString> addDirectory(QString dir);
private:
	QList<QString> m_fileList;
};


#endif // VIDEODB_H
