#include <QtCore/QCoreApplication>
#include <QVariant>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QDebug>


#include <QDateTime>
#include "QTNetString.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    QMap<QString, QVariant> map;
    map["one"] = QVariant(1);
    map["pi"] = QVariant(3.14);
    map["test_german"] = QVariant("Das ist ein test");
    map["character"] = QVariant(QChar('u'));
    map["longtext"] = QVariant("Finally, the general rule is senders should "
                            "have to completely specify the size of what they send "
                            "and receivers should be ready to reject it. If you allow "
                            "arbitrary streaming then your servers will "
                            "suffer attacks that eat your resources. ");
    map["nothing"] = QVariant();
    map["time"] = QVariant(QDateTime::currentDateTime());

    QList<QVariant> list;
    list.append(QVariant("cat"));
    list.append(QVariant("dog"));
    list.append(QVariant("hamster"));
    map["pets"] = list;
    QVariant value(map);


    bool ok;
    QByteArray tns = QTNetString::dump(value, ok);
    if (!ok) {
        qDebug() << "DUMP NOT OK\n";
    }
    else {
        // add a few extra characters after the end of the tns
        // the parser should ignore this characters
        tns.append("Ignore this !!!");

        qDebug() << tns << "\n";

        qDebug() << QTNetString::parse(tns, ok);
        if (!ok) {
            qDebug() << "DUMP NOT OK\n";
        }

    }

    a.exit();
}
