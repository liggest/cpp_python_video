
#include <iostream>

#include <QThread>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "QZeroMQServer.h"
#include "AudioPlayer.h"

QZeroMQServer::QZeroMQServer(QObject* parent) : QObject(parent) { }

void QZeroMQServer::printVersion() {
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    std::cout << "Current 0MQ version is " << major << "." << minor << "." << patch << std::endl;
}

void QZeroMQServer::startRead() {
    std::cout << "Ready to Read!" << std::endl;
    readyToRead = true;
}

void QZeroMQServer::serve() {
    printVersion();
    //  Prepare our context and socket
    zmq::context_t context(2);
    zmq::socket_t socket(context, zmq::socket_type::pair);
    std::cout << "Setting up audio server..." << std::endl;
    socket.bind("ipc://../py/audio_player.ipc");
    //socket.bind("tcp://localhost:5555");

    const char* audio_path = "../resource/audio.mp3";
    
    int sampleRate, channels;
    long long totalSamples;

    zmq::message_t helloMsg;
    zmq::recv_result_t helloResult;
    while (true)
    {
        std::cout << "Receiving Hello..." << std::endl;
        helloResult = socket.recv(helloMsg);
        if (!(helloResult.has_value() && helloMsg.to_string_view().find_first_of("HELLO ") == 0)) {
            //std::this_thread::sleep_for(std::chrono::seconds(1));  // 每秒尝试一次
            QThread::sleep(1);
            continue;
        }
        
        std::cout << "Sending OK..." << std::endl;
        std::string okString = "OK ";
        okString.append(audio_path); // "OK {audio_path}"
        zmq::message_t okMsg(okString);

        if (!socket.send(okMsg, zmq::send_flags::none)) {
            continue;
        }

        zmq::message_t okAnswer;
        zmq::recv_result_t okResult;
            
        std::cout << "Receiving Ready..." << std::endl;
        okResult = socket.recv(okAnswer);
        if (!(okResult.has_value() && okAnswer.to_string_view().find_first_of("READY ") == 0)) {
            continue;
        }

        std::cout << "Get Ready!" << std::endl;

        std::stringstream sStream(okAnswer.to_string().substr(strlen("READY ")));
        sStream >> sampleRate >> totalSamples >> channels;

        break;

    }
    
    long long readSamples = 0;
    long long currentTimeSec = 0;
    long samplesPreRead = sampleRate / 2;
    const long readThreshold = sampleRate;

    AudioPlayer audioPlayer;
    QThread audioThread;
    audioPlayer.moveToThread(&audioThread);

    connect(this, &QZeroMQServer::init, &audioPlayer, &AudioPlayer::init, Qt::ConnectionType::BlockingQueuedConnection);
    //connect(this, &QZeroMQServer::readData, &audioPlayer, &AudioPlayer::readData, Qt::ConnectionType::BlockingQueuedConnection);
    connect(this, &QZeroMQServer::readData, &audioPlayer, &AudioPlayer::readData);
    connect(&audioPlayer, &AudioPlayer::startRead, this, &QZeroMQServer::startRead);

    audioThread.start();

    std::cout << "Wait for Init..." << std::endl;
    int initError = emit QZeroMQServer::init(sampleRate, channels);

    if (initError) {
        std::cerr << "Error during initializing AudioSink" << std::endl;
        return;
    }

    //while (initResult != 0)
    //{
    //    std::cout << "Wait for Reading..." << std::endl;
    //    QThread::msleep(200);
    //}
    std::cout << "Start Reading..." << std::endl;

    std::string readString = "READ ";
    readString.append(std::to_string(samplesPreRead)); // "OK {audio_path}"
    zmq::recv_result_t readResult;

    auto startTime = std::chrono::system_clock::now();

    while (readSamples < totalSamples)
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now - startTime;
        long long secondsPassed = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        if (secondsPassed * sampleRate / 1000 < readSamples - readThreshold)  // 还有的播，就先不读
        {
            QThread::msleep(100);
            continue;
        }

        zmq::message_t readMsg(readString);  // 在循环外创建的话发过消息之后会变成 ""
        std::cout << "Sending Read..." << readString << std::endl;
        socket.send(readMsg, zmq::send_flags::none);
        
        //zmq::multipart_t parts;
        std::vector<zmq::message_t> parts;
        //readResult = zmq::recv_multipart(socket, &parts);
        readResult = zmq::recv_multipart(socket, std::back_inserter(parts));
        if (!readResult) {
            std::cout << "Bad Multiparts" << std::endl;
            continue;
        }
        zmq::message_t &dataMsg = parts.at(0);
        unsigned long long timeNs;

        // 将字节数组复制到变量中
        std::memcpy(&timeNs, parts.at(0).data(), sizeof(timeNs));

        std::cout << "Received Time " << timeNs << std::endl;

        emit QZeroMQServer::readData((char*)dataMsg.data(), dataMsg.size(), timeNs);

        readSamples += samplesPreRead;
        //currentTimeSec = readSamples / sampleRate;
    }

    audioThread.wait();
}
