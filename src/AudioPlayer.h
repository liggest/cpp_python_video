#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <qobject.h>
#include <QtMultimedia>

class AudioPlayer : public QObject
{
	Q_OBJECT
public:
	explicit AudioPlayer(QObject* parent = nullptr);
	~AudioPlayer();

	QAudioSink* sink = nullptr;
	QIODevice* audioIO = nullptr;

	long long totalLatency = 0;

public slots:
	int init(int sampleRate, int channels);
	void readData(const char* data, size_t size, std::chrono::nanoseconds timeNsPY, long readTimes);
	void readByteArray(QByteArray& data, std::chrono::nanoseconds timeNsPY, long readTimes);

//signals:
//	void startRead();

private:
	std::chrono::system_clock::time_point now;
	std::chrono::nanoseconds timeNs;
};


#endif
