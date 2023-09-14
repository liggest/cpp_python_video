#ifndef QZEROMQIOCLIENT_H
#define QZEROMQIOCLIENT_H

#include <qobject.h>

#include "QZeroMQClient.h"

class QZeroMQIOClient : public QZeroMQClient
{
	Q_OBJECT

public:

	void serve();
};


#endif