#ifndef QZEROMQCLIENT_H
#define QZEROMQCLIENT_H

#include <qobject.h>

#include "ZeroMQHelper.h"

class QZeroMQClient : public QObject
{
	Q_OBJECT

public:
	explicit QZeroMQClient(QObject* parent = nullptr);

	void serve();

signals:
	int init(int sampleRate, int channels);
	void readData(const char* data, size_t size, std::chrono::nanoseconds timeNsPY, long readTimes);
	void readByteArray(QByteArray& data, std::chrono::nanoseconds timeNsPY, long readTimes);

protected:
	zmq::socket_t messageSocket;
	zmq::socket_t audioSocket;

	std::string post(std::string message);
	std::string getValue(std::string name);
	static bool startsWith(std::string s, std::string prefix);
};


#endif