#ifndef AUDIOINPUTTER_H
#define AUDIOINPUTTER_H

#include <qobject.h>
#include <QtMultimedia>
#include <QBuffer>

class AudioInputter : public QObject {
	Q_OBJECT

public:
	explicit AudioInputter(QObject* parent = nullptr);
	~AudioInputter();

	QAudioSource* source = nullptr;
	QBuffer* audioIO = nullptr;
	QByteArray* buffer = nullptr;

public slots:
	int init(int sampleRate, int channels);
	//void readData(size_t size);

};

#endif