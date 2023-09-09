#include <iostream>

#include "AudioPlayer.h"

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent) { }

AudioPlayer::~AudioPlayer()
{
	sink->stop();
	delete sink;
}

int AudioPlayer::init(int sampleRate, int channels) {
	std::cout << "Initing AudioSink" << std::endl;
	QAudioDevice device = QMediaDevices::audioOutputs().first();
	if (device.isNull()) {  // 不知起不起作用
		std::cerr << "No Default Device!" << std::endl;
		return 1;
	}
	std::cout << "Default Device Name: " << device.description().toStdString() << std::endl;
	QAudioFormat format;
	format.setSampleRate(sampleRate);
	format.setChannelCount(channels);
	format.setSampleFormat(QAudioFormat::SampleFormat::Int16);
	//format.setSampleFormat(QAudioFormat::SampleFormat::Float);
	if (!device.isFormatSupported(format)) {
		std::cerr << "Unsupported Audio Format!" << std::endl;
		return 1;
	}
	std::cout << "Format Channels: " << format.channelCount() << std::endl;
	std::cout << "Format Sampling Rate: " << format.sampleRate() << std::endl;
	
	sink = new QAudioSink(device, format, this);
    sink->setBufferSize(96000);
	audioIO = sink->start();

	std::cout << "Buffer Size: " << sink->bufferSize() << std::endl;
	std::cout << "AudioSink is Ready" << std::endl;

	//emit AudioPlayer::startRead();
	return 0;
}

void AudioPlayer::readData(const char* data, size_t size, long long timeNs) {
	std::cout << "Reading " << size << " Bytes" << std::endl;
	//QByteArray audioData(data, size);
	//audioIO->write(audioData);
	audioIO->write(data, size);
}


