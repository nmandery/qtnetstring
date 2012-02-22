#ifndef __qtnetstring_h__
#define __qtnetstring_h__


#include "QByteArray"
#include "QVariant"


/**
 * implementation of the "tagged netstring" searialization
 * format
 *
 * http://tnetstrings.org
 */
namespace QTNetString {

    QByteArray dump(const QVariant &value, bool &ok);

}


#endif
