#ifndef QZEROMQIOCLIENT_H
#define QZEROMQIOCLIENT_H

#include <qobject.h>

#include "QZeroMQClient.h"

class QZeroMQIOClient : public QZeroMQClient
{
	Q_OBJECT

public:

	void serve();

protected:

	zmq::socket_t audioSendSocket;
	zmq::socket_t audioReceiveSocket;

	std::string getValue(std::string name);
};


#endif