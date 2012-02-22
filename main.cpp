#include <QtCore/QCoreApplication>
#include <QVariant>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QDebug>


#include <QDateTime>
#include "qtnetstring.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    QMap<QString, QVariant> map;
    map["one"] = QVariant(1);
    map["pi"] = QVariant(3.14);
    map["three"] = QVariant("Das ist ein test");
    map["seven"] = QVariant('u');
    map["sadn"] = QVariant();
    map["time"] = QVariant(QDateTime::currentDateTime());

    QList<QVariant> list;
    list.append(QVariant("item1"));
    map["items"] = list;

    QVariant value(map);

    bool ok;
    qDebug() << QTNetString::dump(value, ok) << "\n";
    if (!ok) {
        qDebug() << "NOT OK\n";
    }

    a.exit();
//    return a.exec();
}
