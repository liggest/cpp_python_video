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
	QAudioDevice device = QMediaDevices::defaultAudioOutput();
	if (device.isNull()) {  // 不知起不起作用
		std::cerr << "No Default Output Device!" << std::endl;
		return 1;
	}
	std::cout << "Default Output Device Name: " << device.description().toLocal8Bit().toStdString() << std::endl;
	QAudioFormat format;
	format.setSampleRate(sampleRate);
	format.setChannelCount(channels);
	format.setSampleFormat(QAudioFormat::SampleFormat::Int16);
	//format.setSampleFormat(QAudioFormat::SampleFormat::Float);
	if (!device.isFormatSupported(format)) {
		std::cerr << "Unsupported Audio Output Format!" << std::endl;
		return 1;
	}
	std::cout << "Output Format Channels: " << format.channelCount() << std::endl;
	std::cout << "Output Format Sampling Rate: " << format.sampleRate() << std::endl;
	
	sink = new QAudioSink(device, format, this);
	sink->setBufferSize(sampleRate * 2);  // 暂时设成采样率的 2 倍 sr 48000 buffer 96000
	std::cout << "Output Buffer Size: " << sink->bufferSize() << std::endl;

	audioIO = sink->start();

	std::cout << "AudioSink is Ready" << std::endl;

	totalLatency = 0;
	//emit AudioPlayer::startRead();
	return 0;
}

void AudioPlayer::readData(const char* data, size_t size, std::chrono::nanoseconds timeNsPY, long readTimes) {
	std::cout << "Reading " << size << " Bytes" << std::endl;
	QByteArray audioData(data, size);
	audioIO->write(audioData);
	//audioIO->write(data, size);
	now = std::chrono::system_clock::now();
	timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
	totalLatency += (timeNs - timeNsPY).count();
	if (readTimes) {
		std::cout << "[Latency (ns)] Total: " << totalLatency << " \tAverage: " << totalLatency / readTimes << " \treadTimes: " << readTimes << std::endl;
	}
}


void AudioPlayer::readByteArray(QByteArray& data, std::chrono::nanoseconds timeNsPY, long readTimes) {
	std::cout << "Reading " << data.size() << " Bytes" << std::endl;
	//QByteArray audioData(data, size);
	audioIO->write(data);
	//data.clear();
	//audioIO->write(data, size);
	now = std::chrono::system_clock::now();
	timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
	totalLatency += (timeNs - timeNsPY).count();
	if (readTimes) {
		std::cout << "[Latency (ns)] Total: " << totalLatency << " \tAverage: " << totalLatency / readTimes << " \treadTimes: " << readTimes << std::endl;
	}
}
