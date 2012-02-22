
#include "qtnetstring.h"

#include <QMap>
#include <QList>
#include <QDebug>

/**
 * (copied from http://tnetstrings.org)
 *
 * About Tagged Netstrings
 * -----------------------
 *
 * TNetStrings stand for a "tagged netstrings" and are a modification of Dan
 * Bernstein's netstrings specification to allow for the same data structures as
 * JSON but in a format that meets these requirements:
 *
 * 1.   Trivial to parse in every language without making errors.
 * 2.   Resistant to buffer overflows and other problems.
 * 3.   Fast and low resource intensive.
 * 4.   Makes no assumptions about string contents and can store binary data
 *      without escaping or encoding them.
 * 5.   Backward compatible with original netstrings.
 * 6.   Transport agnostic, so it works with streams, messages, files, anything
 *      that's 8-bit clean.
 *
 *
 * Grammar
 * -------
 *
 * The grammar for the protocol is simply:
 *
 *      SIZE = [0-9]{1,9}
 *      COLON = ':'
 *      DATA = (.*)
 *      TYPE = ('#' | '}' | ']' | ',' | '!' | '~' | '^')
 *      payload = (SIZE COLON DATA TYPE)+
 *
 * Each of these elements is defined as:
 *
 * SIZE
 *      A ascii encoded integer that is no longer than 9 digits long, and anyone
 *      receiving a message can abort at any length lower than that limit.
 * COLON
 *      A colon character.
 * DATA
 *      A sequence of bytes that is SIZE in length and can include all of the TYPE
 *      chars since the SIZE is used, not the terminal TYPE char.
 * TYPE
 *      A character indicating what type the DATA is.
 *
 * Each TYPE is used to determine the contents and maps to:
 *
 * ,    string (byte array)
 * #    integer
 * ^    float
 * !    boolean of 'true' or 'false'
 * ~    null always encoded as 0:~
 * }    Dictionary which you recurse into to fill with key=value pairs inside
 *      the payload contents.
 * ]    List which you recurse into to fill with values of any type.
 *
 * Implementation Restrictions
 * ---------------------------
 *
 * You are not allowed to implement any of the following features:
 *
 * UTF-8 Strings
 *     String encoding is an application level, political, and display specification.
 *     Transport protocols should not have to decode random character encodings
 *     accurately to function properly.
 * Arbitrary Dict Keys
 *      Keys must be strings only.
 * Floats Undefined
 *      Floats are encoded with X.Y format, with no precision, accuracy, or other assurances.
 *
 * These restrictions exist to make the protocol reliable for anyone who uses it and to
 * act as a constraint on the design to keep it simple.
 *
 */


using namespace QTNetString;

enum TnsType {
    TNS_BOOL        = '!',
    TNS_DICT        = '}',
    TNS_FLOAT       = '^',
    TNS_INT         = '#',
    TNS_LIST        = ']',
    TNS_NULL        = '~',
    TNS_STRING      = ','
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

/**
 * try to convert the value to a bytearray somehow
 * and give it a string represantation in the tnetstring
 */
inline void
dump_unknown(const QVariant &value, QByteArray & tns_value, TnsType & tns_type, bool &ok)
{
    if (value.canConvert(QVariant::String) && !value.canConvert(QVariant::ByteArray)) {
        QString str_value = value.toString();
        tns_value = str_value.toAscii();
        tns_type = TNS_STRING;
    }
    else if (value.canConvert(QVariant::ByteArray)) {
        tns_value = value.toByteArray();
        tns_type = TNS_STRING;
    }
    else {
        qDebug() << "Unsupported variant type: " << value.type();
        ok = false;
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
                dump_unknown(value, tns_value, tns_type, ok);
        }
    }

    if (ok) {
        tns.append(QByteArray::number(tns_value.size()));
        tns.append(':');
        tns.append(tns_value);
        tns.append(QChar(tns_type));
    }

    return tns;
}

