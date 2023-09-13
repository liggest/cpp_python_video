#include <iostream>

#include <QThread>

#include "QZeroMQClient.h"
#include "AudioPlayer.h"

QZeroMQClient::QZeroMQClient(QObject* parent) : QObject(parent) { }

bool startsWith(std::string s, std::string prefix) {
	return s._Starts_with(prefix);
}

void QZeroMQClient::serve() {
	ZeroMQHelper::printVersion();

	zmq::context_t context;
	messageSocket = zmq::socket_t(context, zmq::socket_type::req);

	std::cout << "Setting up Message Client..." << std::endl;
	messageSocket.connect("ipc://../py/temp/message.ipc");

	int sampleRate, channels;
	long long totalSamples;

	std::string status = post("STATUS");
	while (status.empty() || startsWith(status, "IDLE"))
	{
		status = post("STATUS");
	}

	if (startsWith(status, "GET")) {
		std::ostringstream values;
		values << "SET";
		std::string names = status.substr(strlen("GET "));
		size_t spaceIdx;
		std::string name, value;
		while (!names.empty())
		{
			std::cout << "names: " << names << std::endl;
			spaceIdx = names.find_first_of(' ');
			if (spaceIdx != std::string::npos) {
				name = names.substr(0, spaceIdx);
				names = names.substr(spaceIdx + 1);
			}
			else {
				name = names;
				names = std::string(); // 最后一个了
			}
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

	std::string setMessage = post("GET AUDIO_INFO");
	std::stringstream sStream(setMessage.substr(strlen("SET AUDIO_INFO ")));
	sStream >> sampleRate;
	sStream.ignore(1);
	sStream >> totalSamples;
	sStream.ignore(1);  
	sStream >> channels;
	std::cout << "sampleRate: " << sampleRate << " totalSamples: " << totalSamples << " channels: " << channels << std::endl;
	//sStream.ignore(1)
	
	AudioPlayer audioPlayer;
	QThread audioThread;
	audioPlayer.moveToThread(&audioThread);

	connect(this, &QZeroMQClient::init, &audioPlayer, &AudioPlayer::init, Qt::ConnectionType::BlockingQueuedConnection);
	connect(this, &QZeroMQClient::readData, &audioPlayer, &AudioPlayer::readData);

	audioThread.start();

	std::cout << "Wait for Init..." << std::endl;
	int initError = emit QZeroMQClient::init(sampleRate, channels);

	if (initError) {
		std::cerr << "Error during initializing AudioSink" << std::endl;
		return;
	}

	std::cout << "Setting up Audio Client..." << std::endl;

	audioSocket = zmq::socket_t(context, zmq::socket_type::sub);
	audioSocket.connect("ipc://../py/temp/audio_publish.ipc");
	audioSocket.set(zmq::sockopt::subscribe, "");

	long long readSamples = 0;
	long long currentTimeSec = 0;
	long long communicationLatency = 0;
	long readTimes = 0;
	auto now = std::chrono::system_clock::now();
	auto timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
	
	std::cout << "Start Reading..." << std::endl;

	post("SET AUDIO_READY TRUE");

	while (readSamples < totalSamples) {
		//std::cout << "Receiving..." << std::endl;
		std::vector<zmq::message_t> parts;
		if (!zmq::recv_multipart(audioSocket, std::back_inserter(parts))) {
			std::cerr << "Bad Multiparts" << std::endl;
			continue;
		}

		//std::cout << "Received..." << std::endl;
		zmq::message_t& dataMsg = parts.at(0);
		uint64_t ns;
		std::memcpy(&ns, parts.at(1).data(), sizeof(ns));

		std::chrono::nanoseconds timeNsPY(ns);
		now = std::chrono::system_clock::now();
		timeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();

		communicationLatency += (timeNs - timeNsPY).count();
		readTimes += 1;
		bool isOut = readSamples / sampleRate > currentTimeSec;
		if (isOut) {
			std::cout << "[Latency (ns)] Communication: " << communicationLatency << " \tAverage: " << communicationLatency / readTimes << " \treadTimes: " << readTimes << std::endl;
			currentTimeSec++;
		}

		emit QZeroMQClient::readData((char*)dataMsg.data(), dataMsg.size(), timeNsPY, isOut ? readTimes : 0);

		readSamples += dataMsg.size() / 4; // int16 (2 byte) + 2 channels (2 byte)
	}

	audioThread.wait();

	std::cout << "Audio Thread End" << std::endl;
}

std::string QZeroMQClient::post(std::string message) {
	zmq::message_t sendMessage(message);
	std::cout << "Send: " << message << std::endl;
	if (!messageSocket.send(sendMessage, zmq::send_flags::none)) {
		std::cerr << "Send Failed" << std::endl;
		return std::string();
	}
	zmq::message_t recvMessage;
	if (!messageSocket.recv(recvMessage)) {
		std::cerr << "Response Reception Failed" << std::endl;
		return std::string();
	}
	std::cout << "Receive Response: " << recvMessage.to_string_view() << std::endl;
	return recvMessage.to_string();
}

std::string QZeroMQClient::getValue(std::string name) {
	if (name == "AUDIO_PATH") {
		std::cout << "Receive Get for AUDIO_PATH" << std::endl;
		return "../resource/audio.mp3";
	}
	return std::string();
}