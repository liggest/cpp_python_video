#include <iostream>

#include "AudioInputter.h"

AudioInputter::AudioInputter(QObject* parent) { }

AudioInputter::~AudioInputter()
{
	source->stop();
	audioIO->close();
	//delete buffer;
	delete audioIO;
	delete source;
}

int AudioInputter::init(int sampleRate, int channels) {
	std::cout << "Initing AudioSource" << std::endl;
	QAudioDevice device = QMediaDevices::defaultAudioInput();
	if (device.isNull()) {  // 不知起不起作用
		std::cerr << "No Default Input Device!" << std::endl;
		return 1;
	}
	std::cout << "Default Input Device Name: " << device.description().toLocal8Bit().toStdString() << std::endl;
	QAudioFormat format;
	format.setSampleRate(sampleRate);
	format.setChannelCount(channels);
	format.setSampleFormat(QAudioFormat::SampleFormat::Int16);
	//format.setSampleFormat(QAudioFormat::SampleFormat::Float);
	if (!device.isFormatSupported(format)) {
		std::cerr << "Unsupported Audio Input Format!" << std::endl;
		return 1;
	}
	std::cout << "Input Format Channels: " << format.channelCount() << std::endl;
	std::cout << "Input Format Sampling Rate: " << format.sampleRate() << std::endl;

	source = new QAudioSource(device, format, this);
	source->setBufferSize(sampleRate * 2);  // 暂时设成采样率的 2 倍 sr 48000 buffer 96000
	std::cout << "Input Buffer Size: " << source->bufferSize() << std::endl;

	//buffer = new QByteArray(sampleRate * 2, 0);
	audioIO = new QBuffer();
	audioIO->open(QBuffer::ReadWrite);

	source->start(audioIO);

	//audioIO = source->start();

	//connect(audioIO, &QIODevice::readyRead, this, [&]() {
	//	std::cout << "QIODevice::readyRead" << std::endl;
	//	std::cout << "Error: " << audioIO->errorString().toLocal8Bit().toStdString() << std::endl;
	//	std::cout << "isOpen: " << audioIO->isOpen() << std::endl;
	//	std::cout << "isReadable: " << audioIO->isReadable() << std::endl;
	//	std::cout << "size: " << audioIO->size() << std::endl;
	//	audioIO->seek(0);
	//	std::cout << "bytesAvailable: " << audioIO->bytesAvailable() << std::endl;
	//	std::cout << "bytesToWrite: " << audioIO->bytesToWrite() << std::endl;
	//	//std::cout << "buffer: " << buffer->size() << std::endl;
	//	});

	//std::cout << "openMode " << audioIO->openMode() << std::endl;   // 33  ReadOnly | Unbuffered
	std::cout << "Read Channel Count " << audioIO->readChannelCount() << std::endl;    // only 1
	std::cout << "Write Channel Count " << audioIO->writeChannelCount() << std::endl;

	std::cout << "AudioSource is Ready" << std::endl;

	return 0;
}
