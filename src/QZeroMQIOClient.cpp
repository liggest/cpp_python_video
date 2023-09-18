#include <iostream>

#include <QThread>
//#include <QMutex>
//#include <QWaitCondition>

#include "QZeroMQIOClient.h"
#include "AudioInputter.h"
#include "AudioPlayer.h"

void QZeroMQIOClient::serve() {
	ZeroMQHelper::printVersion();

	zmq::context_t context;
	messageSocket = zmq::socket_t(context, zmq::socket_type::req);

	std::cout << "Setting up Message Client..." << std::endl;
	messageSocket.connect("ipc://../py/temp/message.ipc");

	int sampleRate = 48000;  // 因为是读麦克风，所以是定好的
	int channels = 2;
	//long long totalSamples;

	//QMutex mutex;
	//QWaitCondition condition;
	//bool ready = false;

	std::cout << "Setting up Audio Sending Server..." << std::endl;
	audioSendSocket = zmq::socket_t(context, zmq::socket_type::pub);
	audioSendSocket.bind("ipc://../py/temp/audio_input_publish.ipc");

	AudioInputter audioInputter;
	QThread audioInputThread;
	audioInputter.moveToThread(&audioInputThread);

	std::cout << "Wait for AudioInputter::init..." << std::endl;

	connect(this, &QZeroMQIOClient::init, &audioInputter, &AudioInputter::init, Qt::ConnectionType::BlockingQueuedConnection);

	audioInputThread.start();

	int initError = emit QZeroMQIOClient::init(sampleRate, channels);

	if (initError) {
		std::cerr << "Error during initializing AudioInputter" << std::endl;
		return;
	}

	audioInputThread.start();

	std::cout << "QIODevice::bytesWritten Connected" << std::endl;

	AudioPlayer audioPlayer;

	QThread* audioOutputThread = QThread::create([&]() {
		std::cout << "Wait for AudioPlayer::init..." << std::endl;

		
		int initError = audioPlayer.init(sampleRate, channels);
		if (initError) {
			std::cerr << "Error during initializing AudioInputter" << std::endl;
			return;
		}

		std::cout << "Setting up Audio Receiving Server..." << std::endl;
		audioReceiveSocket = zmq::socket_t(context, zmq::socket_type::sub);
		audioReceiveSocket.connect("ipc://../py/temp/audio_publish.ipc");
		audioReceiveSocket.set(zmq::sockopt::subscribe, "");

		auto now = std::chrono::system_clock::now();
		auto startTime = now;
		long startSeconds = 0;
		auto timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
		long readTimes = 0;
		long long communicationLatency = 0;
		uint64_t ns;
		bool isOut = false;

		std::cout << "Starting Audio Receiving Loop..." << std::endl;

		while (true)
		{
				//std::cout << "Receiving..." << std::endl;
				std::vector<zmq::message_t> parts;
				if (!zmq::recv_multipart(audioReceiveSocket, std::back_inserter(parts))) {
					std::cerr << "Bad Multiparts" << std::endl;
					continue;
				}

				//std::cout << "Received Output Audio..." << std::endl;
				zmq::message_t& dataMsg = parts.at(0);
				
				std::memcpy(&ns, parts.at(1).data(), sizeof(ns));

				std::chrono::nanoseconds timeNsPY(ns);
				now = std::chrono::system_clock::now();
				timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();

				communicationLatency += (timeNs - timeNsPY).count();
				readTimes += 1;
				isOut = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count() > startSeconds;
				if (isOut) {
					std::cout << "[Latency (ns)] Output: " << communicationLatency << " \tAverage: " << communicationLatency / readTimes << " \treadTimes: " << readTimes << std::endl;
					startSeconds++;
				}

				audioPlayer.readData((char*)dataMsg.data(), dataMsg.size(), timeNsPY, isOut ? readTimes : 0);
				//emit QZeroMQClient::readData((char*)dataMsg.data(), dataMsg.size(), timeNsPY, isOut ? readTimes : 0);


				//readSamples += dataMsg.size() / 4; // int16 (2 byte) + 2 channels (2 byte)
		}

		});

	audioPlayer.moveToThread(audioOutputThread);
	audioOutputThread->start();

	std::string status = post("STATUS");
	while (status.empty() || startsWith(status, "IDLE"))
	{
		status = post("STATUS");
	}

	if (startsWith(status, "GET")) {
		std::istringstream names(status);
		std::ostringstream values;
		values << "SET";
		std::string name, value;
		names >> name; // 除去 GET
		while (names >> name)
		{
			std::cout << "name: " << name << std::endl;
			value = getValue(name);
			if (value.empty()) continue;
			values << " " << name << " ";
			values << value;
		}
		std::cout << "values: " << values.str() << std::endl;
		post(values.str());
	}

	status = post("STATUS");
	while (status.empty() || !startsWith(status, "PLAYING"))
	{
		status = post("STATUS");
	}

	auto now = std::chrono::system_clock::now();
	//auto startTime = now;
	//long startSeconds = 0;
	auto timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
	long readTimes = 0;
	//long long communicationLatency = 0;

	long long timeNsLong = 0;
	char timeBytes[sizeof(long long)]{};

	audioInputThread.connect(audioInputter.audioIO, &QIODevice::bytesWritten, &audioInputter, [&](size_t size) {
		audioInputter.audioIO->seek(0);
		QByteArray data = audioInputter.audioIO->read(size);
		audioInputter.audioIO->seek(0);
		if (data.size()) {
			now = std::chrono::system_clock::now();
			timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
			readTimes++;

			timeNsLong = timeNs.count();
			std::memcpy(timeBytes, &timeNsLong, sizeof(long long));

			//if (std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count() > startSeconds) {
			//	std::cout << "[Latency (ns)] Input: " << communicationLatency << " \tAverage: " << communicationLatency / readTimes << " \treadTimes: " << readTimes << std::endl;
			//	startSeconds++;
			//}

			std::array<zmq::const_buffer, 2> sendMsgs = {
				zmq::buffer(data.constData(), data.size()),
				zmq::buffer(timeBytes,  sizeof(long long))
			};
			if (!zmq::send_multipart(audioSendSocket, sendMsgs)) {
				std::cerr << "Failed to Send Multiparts" << std::endl;
				return;
			}
			//std::cout << "Send Input Audio..." << std::endl;
		}
		});
	//std::string setMessage = post("GET AUDIO_INFO");
	//std::stringstream sStream(setMessage.substr(strlen("SET AUDIO_INFO ")));
	//sStream >> sampleRate;
	//sStream.ignore(1);
	//sStream >> totalSamples;
	//sStream.ignore(1);  
	//sStream >> channels;
	//std::cout << "sampleRate: " << sampleRate << " totalSamples: " << totalSamples << " channels: " << channels << std::endl;
	//sStream.ignore(1)

	//connect(this, &QZeroMQIOClient::init, &audioPlayer, &AudioPlayer::init, Qt::ConnectionType::BlockingQueuedConnection);
	//connect(this, &QZeroMQIOClient::readData, &audioPlayer, &AudioPlayer::readData);
	//connect(this, &QZeroMQIOClient::readByteArray, &audioPlayer, &AudioPlayer::readByteArray);

	//auto now = std::chrono::system_clock::now();
	//auto startTime = now;
	//auto timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
	//long readTimes = 0;
	//long startSeconds = 0;

	//audioInputThread.connect(audioInputter.audioIO, &QIODevice::readyRead, this, [&]() {});
	//audioInputThread.connect(audioInputter.audioIO, &QIODevice::bytesWritten, &audioInputter, [&](size_t size) {
	//	//std::cout << "QIODevice::bytesWritten" << std::endl;
	//	audioInputter.audioIO->seek(0);
	//	QByteArray data = audioInputter.audioIO->read(size);
	//	audioInputter.audioIO->seek(0);
	//	//QByteArray data = audioInputter.audioIO->data();
	//	//QByteArray* data = audioInputter.buffer;
	//	//std::cout << "Data Input!\t" << data.size() << std::endl;
	//	if (data.size()) {
	//		//emit QZeroMQIOClient::readData(data.constData(), data.size(), timeNs, 0);
	//		now = std::chrono::system_clock::now();
	//		timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
	//		readTimes ++;
	//		if (std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count() > startSeconds) {
	//			emit QZeroMQIOClient::readByteArray(data, timeNs, readTimes);
	//			startSeconds++;
	//		}
	//		else {
	//			emit QZeroMQIOClient::readByteArray(data, timeNs, 0);
	//		}
	//	}
	//	});


	//std::cout << "Setting up Audio Client..." << std::endl;

	//audioSocket = zmq::socket_t(context, zmq::socket_type::sub);
	//audioSocket.connect("ipc://../py/temp/audio_publish.ipc");
	//audioSocket.set(zmq::sockopt::subscribe, "");

	//long long readSamples = 0;
	//long long currentTimeSec = 0;
	//long long communicationLatency = 0;
	//long readTimes = 0;
	//auto now = std::chrono::system_clock::now();
	//auto timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
	//
	//std::cout << "Start Reading..." << std::endl;

	//post("SET AUDIO_READY TRUE");

	//while (readSamples < totalSamples) {
	//	//std::cout << "Receiving..." << std::endl;
	//	std::vector<zmq::message_t> parts;
	//	if (!zmq::recv_multipart(audioSocket, std::back_inserter(parts))) {
	//		std::cerr << "Bad Multiparts" << std::endl;
	//		continue;
	//	}

	//	//std::cout << "Received..." << std::endl;
	//	zmq::message_t& dataMsg = parts.at(0);
	//	uint64_t ns;
	//	std::memcpy(&ns, parts.at(1).data(), sizeof(ns));

	//	std::chrono::nanoseconds timeNsPY(ns);
	//	now = std::chrono::system_clock::now();
	//	timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();

	//	communicationLatency += (timeNs - timeNsPY).count();
	//	readTimes += 1;
	//	bool isOut = readSamples / sampleRate > currentTimeSec;
	//	if (isOut) {
	//		std::cout << "[Latency (ns)] Communication: " << communicationLatency << " \tAverage: " << communicationLatency / readTimes << " \treadTimes: " << readTimes << std::endl;
	//		currentTimeSec++;
	//	}

	//	emit QZeroMQClient::readData((char*)dataMsg.data(), dataMsg.size(), timeNsPY, isOut ? readTimes : 0);

	//	readSamples += dataMsg.size() / 4; // int16 (2 byte) + 2 channels (2 byte)
	//}

	//post("SET END TRUE");

	std::cout << "Waiting Threads" << std::endl;

	audioInputThread.wait();
	audioOutputThread->wait();

	std::cout << "Audio Thread End" << std::endl;
}

std::string QZeroMQIOClient::getValue(std::string name) {
	if (name == "AUDIO_READY") {
		std::cout << "Receive Get for " << name << std::endl;
		return "TRUE";
	}
	return std::string();
}