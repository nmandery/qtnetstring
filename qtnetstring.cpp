#include "qtnetstring.h"
#include <QMap>
#include <QList>
#include <QDebug>

using namespace QTNetString;

enum TnsType {
    TNS_BOOL = '!',
    TNS_DICT = '}',
    TNS_LIST = ']',
    TNS_INT = '#',
    TNS_FLOAT = '^',
    TNS_NULL ='~',
    TNS_STRING = ','
};


inline void
dump_int(const QVariant &value, QByteArray & tns_value, TnsType & tns_type, bool &ok)
{
    ok = value.canConvert(QVariant::ByteArray);
    if (ok) {
        tns_value = value.toByteArray();
        tns_type = TNS_INT;
    }
}

inline void
dump_string(const QVariant &value, QByteArray & tns_value, TnsType & tns_type, bool &ok)
{
    ok = value.canConvert(QVariant::ByteArray);
    if (ok) {
        tns_value = value.toByteArray();
        tns_type = TNS_STRING;
    }
}


inline void
dump_float(const QVariant &value, QByteArray & tns_value, TnsType & tns_type, bool &ok)
{
    ok = value.canConvert(QVariant::ByteArray);
    if (ok) {
        tns_value = value.toByteArray();
        tns_type = TNS_FLOAT;
    }
}

inline void
dump_bool(const QVariant &value, QByteArray & tns_value, TnsType & tns_type, bool &ok)
{
    ok = value.canConvert(QVariant::Bool);
    if (ok) {
        if (value.toBool()) {
            tns_value = "true";
        }
        else {
            tns_value = "false";
        }
        tns_type = TNS_BOOL;
    }
}


inline void
dump_map(const QVariant &value, QByteArray & tns_value, TnsType & tns_type, bool &ok)
{
    ok = value.canConvert(QVariant::Map);
    if (ok) {
        QMap<QString, QVariant> map_value = value.toMap();
        QMap<QString, QVariant>::const_iterator iter = map_value.constBegin();
        while (iter != map_value.constEnd() && ok ) {
            QString key_str = iter.key();
            QVariant key(key_str);
            tns_value.append(dump(key, ok));
            if (ok) {
                tns_value.append(dump(iter.value(), ok));
            }
            ++iter;
        }
        tns_type = TNS_DICT;
    }
}


inline void
dump_list(const QVariant &value, QByteArray & tns_value, TnsType & tns_type, bool &ok)
{
    ok = value.canConvert(QVariant::List);
    if (ok) {
        QList<QVariant> list_value = value.toList();
        QList<QVariant>::const_iterator iter = list_value.constBegin();
        while (iter != list_value.constEnd() && ok ) {
            QVariant key(*iter);
            tns_value.append(dump(key, ok));
            ++iter;
        }
        tns_type = TNS_LIST;
    }
}


inline void
dump_time(const QVariant &value, QByteArray & tns_value, TnsType & tns_type, bool &ok)
{
    ok = value.canConvert(QVariant::String);
    if (ok) {
        QString time_value = value.toString();
        QVariant time_v(time_value);
        dump_string(time_v, tns_value, tns_type, ok);
    }
}


QByteArray
QTNetString::dump(const QVariant &value, bool &ok)
{
    QByteArray tns;
    QByteArray tns_value;
    TnsType tns_type = TNS_NULL;
    ok = true;

    if (!value.isNull()) {
        switch(value.type()) {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::ULongLong:
                dump_int(value, tns_value, tns_type, ok);
                break;
            case QVariant::Char:
            case QVariant::String:
            case QVariant::ByteArray:
                dump_string(value, tns_value, tns_type, ok);
                break;
            case QVariant::Time:
            case QVariant::DateTime:
            case QVariant::Date:
                dump_time(value, tns_value, tns_type, ok);
                break;
            case QVariant::Double:
                dump_float(value, tns_value, tns_type, ok);
                break;
            case QVariant::Bool:
                dump_bool(value, tns_value, tns_type, ok);
                break;
            case QVariant::List:
                dump_list(value, tns_value, tns_type, ok);
                break;
            case QVariant::Map:
            case QVariant::Hash:
                dump_map(value, tns_value, tns_type, ok);
                break;
            default:
                qDebug() << "Unsupported variant type: " << value.type();
                ok = false;
        }
    }

    if (ok) {
        tns.append(QByteArray::number(tns_value.size()));
        tns.append(":");
        tns.append(tns_value);
        tns.append(QChar(tns_type));
    }

    return tns;
}

