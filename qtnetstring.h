#ifndef __qtnetstring_h__
#define __qtnetstring_h__


#include "QByteArray"
#include "QVariant"


namespace QTNetString {

    QByteArray dump(const QVariant &value, bool &ok);

}


#endif
