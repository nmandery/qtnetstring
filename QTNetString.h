#ifndef __qtnetstring_h__
#define __qtnetstring_h__


#include "QByteArray"
#include "QVariant"


/**
 * Implementation of the "tagged netstring" searialization
 * format
 *
 * http://tnetstrings.org
 */
namespace QTNetString {

    /**
     * Dump the contents of a QVariant structure into
     * a QByteArray.
     *
     * The QVariants will be serialized to their TNetString
     * counterpart. All other QVaraints which support a cast
     * to QByteArray or QString will be serializied to TNetString
     * string types.
     *
     * sets ok to false in case of an error and return
     * a empty QByteArray.
     */
    QByteArray dump(const QVariant &value, bool &ok);

    /**
     * Parse the contents of the given TNetString and
     * return its contents as a QVariant structure.
     *
     * returns QVariant::Invalid on error and sets ok
     * to false.
     */
    QVariant parse(const QByteArray &tnetstring, bool &ok);

    /**
     * the same as the parse method with the difference
     * that pasinf starts at tns_start_pos and all characters
     * before this position are ignored.
     *
     * Parsing will continue until either the end of the tns structure
     * or the end of the tnetstring is reached.
     *
     * the position of the last character of the tns will be written
     * to the tns_end_pos parameter
     */
    QVariant parse(const QByteArray &tnetstring, int tns_start_pos, int &tns_end_pos, bool &ok);

}


#endif
