#ifndef QZEROMQSERVER_H
#define QZEROMQSERVER_H

#include <qobject.h>

class QZeroMQServer : public QObject
{
	Q_OBJECT

public:
	explicit QZeroMQServer(QObject* parent = nullptr);

	void printVersion();

	void serve();

public slots:
	void startRead();

signals:
	int init(int sampleRate, int channels);
	void readData(const char* data, size_t size, long long timeNs);

private:
	bool readyToRead = false;
};


#endif
