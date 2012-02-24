
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
 *      Floats are encoded with X.Y format, with no precision, accuracy, or other
 *      assurances.
 *
 * These restrictions exist to make the protocol reliable for anyone who uses it and to
 * act as a constraint on the design to keep it simple.
 *
 */


using namespace QTNetString;

/* neccessary prototypes */
QVariant parse_payload(const QByteArray &payload, int start_pos, int end_pos,
            int &this_end_pos, bool &ok);

enum TnsType {
    TNS_BOOL        = '!',
    TNS_MAP        = '}',
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


/**
 * dump QChar as QStrings
 */
inline void
dump_char(const QVariant &value, QByteArray & tns_value, TnsType & tns_type, bool &ok)
{
    ok = (value.type() == QVariant::Char) && (value.canConvert(QVariant::String));
    if (ok) {
        tns_value = value.toString().toAscii();
        tns_type = TNS_STRING;
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
        tns_type = TNS_MAP;
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
            case QVariant::LongLong:
                dump_int(value, tns_value, tns_type, ok);
                break;
            case QVariant::Char:
                dump_char(value, tns_value, tns_type, ok);
                break;
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


inline void
parse_bool(const QByteArray &payload, QVariant &value, int pl_start, int pl_size)
{
    QByteArray pl_data = payload.mid(pl_start, pl_size);
    value.setValue(pl_data == "true");
}


/**
 * TODO: cast into QVariant of appropriate size
 */
inline void
parse_int(const QByteArray &payload, QVariant &value, int pl_start, int pl_size, bool & ok)
{
    QByteArray pl_data = payload.mid(pl_start, pl_size);
    value.setValue(pl_data.toInt(&ok));
    if (!ok) {
        qDebug() << "could not convert to int";
    }
}

inline void
parse_string(const QByteArray &payload, QVariant &value, int pl_start, int pl_size)
{
    QByteArray pl_data = payload.mid(pl_start, pl_size);
    value.setValue(pl_data);
}


inline void
parse_float(const QByteArray &payload, QVariant &value, int pl_start, int pl_size, bool & ok)
{
    QByteArray pl_data = payload.mid(pl_start, pl_size);
    value.setValue(pl_data.toDouble(&ok));
    if (!ok) {
        qDebug() << "could not convert to float";
    }
}

/**
 * parse the contents of a list
 */
inline void
parse_list(const QByteArray &payload, QVariant &value, int pl_start, int pl_size, bool & ok)
{
    QList<QVariant> list;

    // set empty list if size is 0
    if (pl_size == 0) {
        value.setValue(list);
        return;
    }

    int this_end_pos = pl_start;
    while (ok && (this_end_pos < (pl_start+pl_size-1))) {
        QVariant list_value = parse_payload(payload, this_end_pos, pl_size+pl_start-1,
                    this_end_pos, ok);

        if (!ok) {
            qDebug() << "list element is not ok";
            break;
        }
        list.append(list_value);
    }

    if (ok) {
        value.setValue(list);
    }
}

/**
 * parse the contents of a map
 */
inline void
parse_map(const QByteArray &payload, QVariant &value, int pl_start, int pl_size, bool & ok)
{
    QMap<QString, QVariant> map;

    // set empty map if size is 0
    if (pl_size == 0) {
        value.setValue(map);
        return;
    }

    int this_end_pos = pl_start;

    while (ok && (this_end_pos < (pl_start+pl_size-1))) {
        QVariant map_key = parse_payload(payload, this_end_pos, pl_size+pl_start-1,
                this_end_pos, ok);
        if (ok) {
            if (map_key.type() == QVariant::ByteArray) {

                QVariant map_value = parse_payload(payload, this_end_pos,
                            pl_size+pl_start-1, this_end_pos, ok);
                if (ok) {
                    // qvariant maps only allow QStrings as keys. Need to 
                    // cast the QByteArray to String
                    map[map_key.toString()] =  map_value;
                }
            }
            else {
                qDebug() << "tns map keys are only allowed to be strings";
                ok = false;
            }
        }
    }

    if (ok) {
        value.setValue(map);
    }
}

/**
 *
 * this_end_pos: last position of this value (= position of tns_type character)
 */
QVariant
parse_payload(const QByteArray &payload, int start_pos, int end_pos, int &this_end_pos, bool &ok)
{
    QVariant value;

    if (payload.size() <= 0) {
        qDebug() << "tns payload is empty";
        ok = false;
        return value;
    }

    if ((start_pos > end_pos) || ((payload.size()-1) < end_pos)) {
        qDebug() << "invalid positions/sizes";
        ok = false;
        return value;
    }

    int colon_pos = payload.indexOf(':', start_pos);
    if ((colon_pos == -1) || (colon_pos > end_pos)) {
        qDebug() << "no seperating colon found";
        ok = false;
        return value;
    }

    // convert the size to int;
    QByteArray ba_size = payload.mid(start_pos, colon_pos-start_pos);
    int pl_size = ba_size.toInt(&ok);
    if (!ok) {
        qDebug() << "invalid tns size: " <<ba_size;
        ok = false;
        return value;
    }

    int pl_start = colon_pos + 1;
    int pl_end = pl_start + pl_size - 1;
    if (pl_end >= end_pos) {
        qDebug() << "tns specifies no type";
        ok = false;
        return value;
    }

    switch (payload.at(pl_end + 1)) {
        case TNS_NULL:
            if (pl_size != 0) {
                qDebug() << "null values must have a size of 0";
            }
            // do not set any value
            break;
        case TNS_STRING:
            parse_string(payload, value, pl_start, pl_size);
            break;
        case TNS_BOOL:
            parse_bool(payload, value, pl_start, pl_size);
            break;
        case TNS_INT:
            parse_int(payload, value, pl_start, pl_size, ok);
            break;
        case TNS_FLOAT:
            parse_float(payload, value, pl_start, pl_size, ok);
            break;
        case TNS_MAP:
            parse_map(payload, value, pl_start, pl_size, ok);
            break;
        case TNS_LIST:
            parse_list(payload, value, pl_start, pl_size, ok);
            break;
        default:
            qDebug() << "unknown tns type: " << payload.at(pl_end + 1);
            ok = false;
    }

    // last position of this value ( = tns_type character)
    this_end_pos = pl_start + pl_size + 1;
    return value;
}


QVariant
QTNetString::parse(const QByteArray &tnetstring, bool &ok)
{
    QVariant value;
    ok = true;

    if (tnetstring.size() > 0) {
        int this_end_pos;
        value = parse_payload(tnetstring, 0, tnetstring.size() - 1, this_end_pos, ok);

        // reset to empty QVariant in case of an error to
        // return type Invalid
        if (!ok) {
            value.clear();
        }
    }
    else {
        qDebug() << "tns is empty";
        ok = false;
    }

    return value;
}
