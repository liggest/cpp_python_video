#ifndef QZEROMQSERVER_H
#define QZEROMQSERVER_H

#include <qobject.h>

class QZeroMQServer : public QObject
{
	Q_OBJECT

public:
	explicit QZeroMQServer(QObject* parent = nullptr);

	void serve();

	//public slots:
	//	void startRead();

signals:
	int init(int sampleRate, int channels);
	void readData(const char* data, size_t size, std::chrono::nanoseconds timeNsPY, long readTimes);

	//private:
	//	bool readyToRead = false;
};


#endif
