#include <iostream>
//#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <codecvt>

#include <Python.h>
#include <numpy/arrayobject.h>
#include <QApplication>
#include <QTimer>
#include <QtMultimedia>

//#include <QThread>
#include <locale>


#include "VideoWidget.h"

#include "PyAPIPlugin.h"

PyAPIPlugin::PyAPIPlugin(QApplication *_app) : app(_app) {}

void PyAPIPlugin::playAudio() {
	//std::string command = "ffplay -vn -nodisp -loglevel quiet -i \"../resource/video.mp4\"";
	//std::system(command.c_str());
	//QAudioDeviceInfo device(QAudioDeviceInfo::defaultOutputDevice());
	//QAudioDevice device = QMediaDevices::defaultAudioOutput();
	//std::cout << "Device Name: " << device.description().toStdString() << std::endl;
	//QAudioFormat format;
	//format.setSampleRate(44100);
	//format.setChannelCount(2);
	//format.setSampleFormat(QAudioFormat::SampleFormat::Int16);
	//if (!device.isFormatSupported(format)) {
	//	std::cout << "Unsupported Audio Format " << std::endl;
	//}
	////QAudioOutput audioOutput(device, format);
	//QAudioSink audioSink(device, format);
	//QIODevice* audioIO = audioSink.start();
	//QIODevice* audioIO = audioOutput.start();
	
}

long msToSamples(long ms, int sampleRate) {
	return ms * sampleRate / 1000;
}

std::wstring getEnvVarAsWstring(const wchar_t* name) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const char* value = std::getenv(converter.to_bytes(name).c_str());
    return value ? converter.from_bytes(value) : L"";
}

void setEnvVarAsWstring(const wchar_t* name, const std::wstring& value) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string name_str = converter.to_bytes(name);
    std::string value_str = converter.to_bytes(value);
#ifdef _WIN32
	_putenv_s(name_str.c_str(), value_str.c_str());
#else
	setenv(name_str.c_str(), value_str.c_str(), 1);
#endif
}

// Get path of .venv
int getPythonPath(std::filesystem::path& pythonPath) {
    std::error_code ec;
    std::filesystem::path cwd = std::filesystem::current_path(ec);
    if (ec) {
        std::cerr << "Cannot get current working directory" << std::endl;
        return 1;
    }

    std::filesystem::path parent = cwd.parent_path();
    std::cout << "parent path: " << parent << std::endl;

    std::filesystem::path pyPath = parent / "py" / ".venv";
    std::cout << "python path: " << pyPath << std::endl;

    pythonPath = pyPath;
    return 0;
}
// Add .venv to PATH
void addPythonPath(const std::filesystem::path& pythonPath) {
	std::wcout.imbue(std::locale(""));
#ifdef _WIN32
    // because windows can run, we don't change
    // Add python path to PATH

	// Get current PATH
	std::wstring path_env = _wgetenv(L"PATH");

	// Print current PATH
	//std::wcout << L"Current PATH: " << path_env << std::endl;
    //wprintf(L"PATH=%s", _wgetenv(L"PATH"));

	// Add python path to PATH
	std::wcout << L"Adding python path to PATH" << std::endl;
	std::wstring envPath;

	envPath = pythonPath.wstring().append(L";").append(path_env);
    //envPath.append(path).append(L";").append(_wgetenv(L"PATH"));

	// Set updated PATH
	_wputenv_s(L"PATH", envPath.c_str());

	// Print updated PATH
	//std::wcout << L"Updated PATH: " << _wgetenv(L"PATH") << std::endl;

    //std::wcout << L"path: " << path << std::endl;
    //std::wcout << L"envPath: " << envPath << std::endl;
    //std::wcout << L"==================================" << std::endl;
    //wprintf(L"PATH=%s", _wgetenv(L"PATH"));
#else
    //std::wcout.imbue(std::locale(""));
    //std::wstring path = pythonPath.wstring();

    // Get current PATH
    std::wstring path_env = getEnvVarAsWstring(L"PATH");

    // Print current PATH
    //std::wcout << L"Current PATH: " << path_env << std::endl;

    // Add python path to PATH
    std::wcout << L"Adding python path to PATH" << std::endl;
    std::wstring envPath;

    envPath = (pythonPath / "lib").wstring() + L":" + path_env;

    // Set updated PATH
    setEnvVarAsWstring(L"PATH", envPath);

    // Print updated PATH
    //std::wcout << L"Updated PATH: " << getEnvVarAsWstring(L"PATH") << std::endl;
#endif

}

void initPython() {
    std::filesystem::path path;
	getPythonPath(path);
	addPythonPath(path);

	PyStatus status;

	PyConfig config;
	PyConfig_InitPythonConfig(&config);

	// Set PYTHONHOME
	status = PyConfig_SetString(&config, &config.home, path.wstring().c_str());
	if (PyStatus_Exception(status)) {
		goto exception;
	}

	// Set PYTHONPATH
	status = PyConfig_SetString(&config, &config.pythonpath_env, path.wstring().c_str());
	if (PyStatus_Exception(status)) {
		goto exception;
	}

	/* Set the program name. Implicitly preinitialize Python. */
//#ifdef _WIN32
//	status = PyConfig_SetString(&config, &config.program_name, wcscat(path, L"\\python"));
//#else
//	status = PyConfig_SetString(&config, &config.program_name, wcscat(path, L"/python"));
//#endif
//	if (PyStatus_Exception(status)) {
//		goto exception;
//	}

	status = Py_InitializeFromConfig(&config);
	if (PyStatus_Exception(status)) {
		goto exception;
	}
	PyConfig_Clear(&config);
	return;

exception:
	PyConfig_Clear(&config);
	Py_ExitStatusException(status);
}

int PyAPIPlugin::run() {
	//Py_SetPythonHome(path);
	//Py_Initialize();
	initPython();
	import_array();

	std::wcout << "PYTHONHOME: " << Py_GetPythonHome() << std::endl;
	std::wcout << "PROGRAM_NAME: " << Py_GetProgramName() << std::endl;
	std::wcout << "PROGRAM_FULL_PATH: " << Py_GetProgramFullPath() << std::endl;
	std::wcout << "EXEC_PREFIX: " << Py_GetExecPrefix() << std::endl;
	std::wcout << "PYTHONPATH: " << Py_GetPath() << std::endl;

	// 导入 Python 模块
	//PyObject *pModule = PyImport_ImportModule("read_video");
	PyObject* pModule = PyImport_ImportModule("video_reader.opencv");
	if (pModule == nullptr) {
		//std::cerr << "Cannot import read_video.py" << std::endl;
		std::cerr << "Cannot import video_reader.opencv" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}

	// 获取模块中的 init_capture 函数
	PyObject* pInitCapture = PyObject_GetAttrString(pModule, "init_capture");
	if (pInitCapture == nullptr) {
		std::cerr << "Cannot find 'init_capture' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}
	// 获取模块中的 read_video 函数
	PyObject* pReadVideo = PyObject_GetAttrString(pModule, "read_video");
	if (pReadVideo == nullptr) {
		std::cerr << "Cannot find 'read_video' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}
	// 获取模块中的 video_size 函数
	PyObject* pVideoSize = PyObject_GetAttrString(pModule, "video_size");
	if (pVideoSize == nullptr) {
		std::cerr << "Cannot find 'video_size' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}
	// 获取模块中的 video_fps 函数
	PyObject* pVideoFrames = PyObject_GetAttrString(pModule, "video_frames");
	if (pVideoFrames == nullptr) {
		std::cerr << "Cannot find 'video_frames' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}
	// 获取模块中的 release_capture 函数
	PyObject* pReleaseCapture = PyObject_GetAttrString(pModule, "release_capture");
	if (pReleaseCapture == nullptr) {
		std::cerr << "Cannot find 'release_capture' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}

	// 调用 init_capture 函数
	PyObject* pVideoPath = PyUnicode_FromString("../resource/video.mp4");
	//PyObject *pCap = PyObject_CallObject(pFunc, PyTuple_Pack(1, pVideoPath));
	PyObject* pCap = PyObject_CallObject(pInitCapture, PyTuple_Pack(1, pVideoPath));

	// 检查是否成功打开视频文件
	//PyObject *pIsOpened = PyObject_CallMethod(pCap, "isOpened", nullptr);
	if (pCap == Py_None) {
		std::cerr << "Cannot open the video file" << std::endl;
		return 1;
	}

	// 创建 VideoWidget
	VideoWidget videoWidget;
	videoWidget.show();

	// 根据视频尺寸设置 VideoWidget 尺寸
	PyObject* pSize = PyObject_CallObject(pVideoSize, nullptr);
	PyObject* pWidth = PyTuple_GetItem(pSize, 0);
	PyObject* pHeight = PyTuple_GetItem(pSize, 1);
	//long width = PyLong_AsLong(pWidth);
	//long height = PyLong_AsLong(pHeight);
	int width = static_cast<int>(PyFloat_AsDouble(pWidth));
	if (width < 0 && PyErr_Occurred()) {
		PyErr_Print();
	}
	int height = static_cast<int>(PyFloat_AsDouble(pHeight));
	if (height < 0 && PyErr_Occurred()) {
		PyErr_Print();
	}
	std::cout << "Video Info:" << std::endl;
	std::cout << "width: " << width << " height: " << height << std::endl;
	if (width > 0 && height > 0) {
		videoWidget.resize(width, height);
	}

	PyObject* pFrameInfo = PyObject_CallObject(pVideoFrames, nullptr);
	PyObject* pFrameCount = PyTuple_GetItem(pFrameInfo, 0);
	PyObject* pFPS = PyTuple_GetItem(pFrameInfo, 1);
	int totalFrames = static_cast<int>(PyFloat_AsDouble(pFrameCount));
	if (totalFrames < 0 && PyErr_Occurred()) {
		PyErr_Print();
	}
	//int fps = PyLong_AsLong(pFPS);
	int fps = static_cast<int>(PyFloat_AsDouble(pFPS));
	if (fps < 0) {
		if (PyErr_Occurred()) PyErr_Print();
		fps = 30;  // 默认 1000 / 30;
	}
	std::cout << "fps: " << fps << " total frames: " << totalFrames << std::endl;
	long long totalSeconds = totalFrames / fps;
	if (totalFrames % fps) {  // 剩余的帧不足一秒，当做一秒算
		totalSeconds += 1;
	}
	std::cout << "Video Calculated Duration: " << totalSeconds / 60 << "m" << totalSeconds % 60 << "s (" << totalSeconds << "s)" << std::endl;

	// 创建 QTimer，用于定时更新视频帧
	//QTimer timer;
	//timer.setInterval(1000 / fps);  // 按 fps 的间隔更新视频帧
	//timer.setInterval(30);  // 按30ms的间隔更新视频帧

	// 导入 Python 模块
	PyObject* pAudioModule = PyImport_ImportModule("video_reader.audio");
	if (pAudioModule == nullptr) {
		//std::cerr << "Cannot import read_video.py" << std::endl;
		std::cerr << "Cannot import video_reader.audio" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}

	// 获取模块中的 init_audio 函数
	PyObject* pInitAudio = PyObject_GetAttrString(pAudioModule, "init_audio");
	if (pInitAudio == nullptr) {
		std::cerr << "Cannot find 'init_audio' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}
	// 获取模块中的 read_audio 函数
	//PyObject* pReadAudio = PyObject_GetAttrString(pAudioModule, "read_audio_bytes");
	//PyObject* pReadAudio = PyObject_GetAttrString(pAudioModule, "read_audio_int16");
	PyObject* pReadAudio = PyObject_GetAttrString(pAudioModule, "read_audio_float32");
	if (pReadAudio == nullptr) {
		std::cerr << "Cannot find 'read_audio_bytes' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}
	// 获取模块中的 audio_samples 函数
	PyObject* pAudioSamples = PyObject_GetAttrString(pAudioModule, "audio_samples");
	if (pAudioSamples == nullptr) {
		std::cerr << "Cannot find 'audio_samples' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}
	// 获取模块中的 audio_finished 函数
	PyObject* pAudioFinished = PyObject_GetAttrString(pAudioModule, "audio_finished");
	if (pAudioFinished == nullptr) {
		std::cerr << "Cannot find 'audio_finished' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}
	// 获取模块中的 release_audio 函数
	PyObject* pReleaseAudio = PyObject_GetAttrString(pAudioModule, "release_audio");
	if (pReleaseAudio == nullptr) {
		std::cerr << "Cannot find 'release_audio' function" << std::endl;
		if (PyErr_Occurred()) PyErr_Print();
		return 1;
	}

	// 调用 init_audio 函数
	PyObject* pAudioPath = PyUnicode_FromString("../resource/audio.mp3");
	PyObject* pAudioFile = PyObject_CallObject(pInitAudio, PyTuple_Pack(1, pAudioPath));
	// 检查是否成功打开音频文件
	if (PyErr_Occurred()) {
		std::cerr << "Cannot open the audio file" << std::endl;
		PyErr_Print();
		return 1;
	}

	// 调用 audio_samples 函数
	PyObject* pAudioInfo = PyObject_CallObject(pAudioSamples, nullptr);
	PyObject* pSampleRate = PyTuple_GetItem(pAudioInfo, 0);
	PyObject* pSamples = PyTuple_GetItem(pAudioInfo, 1);
	PyObject* pChannels = PyTuple_GetItem(pAudioInfo, 2);
	int sampleRate = PyLong_AsLong(pSampleRate);
	if (PyErr_Occurred()) {
		PyErr_Print();
		return 1;
	}
	long long totalSamples = PyLong_AsLong(pSamples);
	if (PyErr_Occurred()) {
		PyErr_Print();
		return 1;
	}
	int channels = PyLong_AsLong(pChannels);
	if (PyErr_Occurred()) {
		PyErr_Print();
		return 1;
	}
	std::cout << "Audio Info:" << std::endl;
	std::cout << "Sample Rate: " << sampleRate << " Channels: " << channels << " Total Samples: " << totalSamples << std::endl;

	//QList devices = QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioOutput);
	QList devices = QMediaDevices::audioOutputs();
	//for each (QAudioDeviceInfo device in devices)
	//{
	//	std::cout << "Device Name: " << device.deviceName().toStdString() << std::endl;
	//}

	//QAudioDeviceInfo device(QAudioDeviceInfo::defaultOutputDevice());
	//QAudioDeviceInfo device = devices.first();
	QAudioDevice device = devices.first();
	//std::cout << "Default Device Name: " << device.deviceName().toStdString() << std::endl;
	std::cout << "Default Device Name: " << device.description().toStdString() << std::endl;
	QAudioFormat format;
	format.setSampleRate(sampleRate);
	//format.setSampleSize(16);
	//format.setSampleType(QAudioFormat::SampleType::SignedInt);
	//format.setSampleFormat(QAudioFormat::SampleFormat::Int16);
	format.setSampleFormat(QAudioFormat::SampleFormat::Float);
	format.setChannelCount(channels);
	//format.setByteOrder(QAudioFormat::LittleEndian);
	//format = device.nearestFormat(format);
	if (!device.isFormatSupported(format)) {
		std::cout << "Unsupported Audio Format!" << std::endl;
		return 1;
	}
	std::cout << "Format Channels: " << format.channelCount() << std::endl;
	std::cout << "Format Sample Rate: " << format.sampleRate() << std::endl;
	//std::cout << "Format Sample Size: " << format.sampleSize() << std::endl;
	//std::cout << "Format Byte Order: " << format.byteOrder() << std::endl;
	//std::cout << "Format Codec: " << format.codec().toStdString() << std::endl;

	auto now = std::chrono::system_clock::now();
	auto nowNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
	auto epochNs = nowNs.time_since_epoch();
	long long timeNs = epochNs.count();
	auto startTime = now;
	auto duration = now - startTime;

	long oneFrameMs = 1000 / fps;
	long long fileTimeMs = 0;

	long long totalLatency = 0;
	long framePassed = 0;

	PyObject* pRetVal = nullptr;
	PyObject* pAudioRetVal = nullptr;

	//std::thread audio_thread(playAudio);

	//QThread* video_thread = QThread::create([&]() {
	//QTimer::singleShot(0, [&]() {
	std::thread video_thread([&]() {
		std::cout << "Thread Starting" << std::endl;
		//QAudioOutput audioOutput(device, format);
		QAudioSink audioSink(device, format);
		//QIODevice* audioIO = audioOutput.start();
		QIODevice* audioIO = audioSink.start();
		Py_BEGIN_ALLOW_THREADS
		PyGILState_STATE gstate = PyGILState_Ensure();  // 获取 GIL
		pAudioRetVal = PyObject_CallObject(pReadAudio, PyTuple_Pack(1, PyLong_FromLong(msToSamples(oneFrameMs, sampleRate))));
		PyObject* pAudioData = PyTuple_GetItem(pAudioRetVal, 0);
		PyObject* pAudioByte = PyArray_ToString((PyArrayObject*)pAudioData, NPY_ORDER::NPY_CORDER);
		//PyObject* pAudioByte = pAudioData;
		//long length = PyArray_SHAPE(reinterpret_cast<PyArrayObject*>(pAudioData))[0];
		//int audioChannels = PyArray_SHAPE(reinterpret_cast<PyArrayObject*>(pAudioData))[1];
		//std::cout  << "Length: " << length << " Channels: " << audioChannels << std::endl;
		//long size = length * audioChannels;

		//const QByteArray audioData = QByteArray::fromRawData((char*)PyArray_BYTES(pAudioArray), size);
		QByteArray audioData(PyBytes_AsString(pAudioByte), PyBytes_Size(pAudioByte));
		audioIO->write(audioData);
		long timeReadMs = oneFrameMs;
		while(true) {
			if (pRetVal != nullptr) {
				Py_DECREF(pRetVal);
			}
			if (pAudioRetVal != nullptr) {
				Py_DECREF(pAudioRetVal);
				Py_XDECREF(pAudioByte);
			}
			//std::cout << "! 1" << std::endl;

			pRetVal = PyObject_CallObject(pReadVideo, nullptr);
			
			PyObject* pIsRead = PyTuple_GetItem(pRetVal, 0);

			if (pIsRead == Py_False) {
				Py_DECREF(pRetVal);
				Py_DECREF(pAudioRetVal);
				Py_XDECREF(pAudioByte);
				PyGILState_Release(gstate);  // 释放 GIL
				now = std::chrono::system_clock::now();
				duration = now - startTime;
				long long seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
				std::cout << "Video Playback Time: " << seconds / 60 << "m" << seconds % 60 << "s (" << seconds << "s)" << std::endl;
				long long fileSeconds = fileTimeMs / 1000;
				std::cout << "Video File Duration: " << fileSeconds / 60 << "m" << fileSeconds % 60 << "s (" << fileSeconds << "s)" << std::endl;
				return; // 视频播放结束
			}

			//std::cout << "! 2" << std::endl;

			PyObject* pFrame = PyTuple_GetItem(pRetVal, 1);
			PyObject* pAimedTimeMs = PyTuple_GetItem(pRetVal, 2);
			PyObject* pTimeNs = PyTuple_GetItem(pRetVal, 3);

			long long newFileTimeMs = static_cast<int>(PyFloat_AsDouble(pAimedTimeMs));
			if (newFileTimeMs < 0 && PyErr_Occurred()) {
				PyErr_Print();
				// TODO 还不知道怎么处理
			}
			
			oneFrameMs = newFileTimeMs - fileTimeMs;
			pAudioRetVal = PyObject_CallObject(pReadAudio, PyTuple_Pack(1, PyLong_FromLong(msToSamples(oneFrameMs, sampleRate))));
			pAudioData = PyTuple_GetItem(pAudioRetVal, 0);
			pAudioByte = PyArray_ToString((PyArrayObject*)pAudioData, NPY_ORDER::NPY_CORDER);
			//pAudioByte = pAudioData;
			//length = PyArray_SHAPE(pAudioArray)[0];
			//audioChannels = PyArray_SHAPE(pAudioArray)[1];
			//size = length * audioChannels;
			QByteArray audioData(PyBytes_AsString(pAudioByte), PyBytes_Size(pAudioByte));
			audioIO->write(audioData);
			timeReadMs += oneFrameMs;
			fileTimeMs = newFileTimeMs;

			// 将 Python 的 numpy.array 转换为 QImage
			PyArrayObject* pFrameArray = reinterpret_cast<PyArrayObject*>(pFrame);
			int rows = PyArray_SHAPE(pFrameArray)[0];
			int cols = PyArray_SHAPE(pFrameArray)[1];
			int channels = PyArray_SHAPE(pFrameArray)[2];

			QImage image((uchar*)PyArray_DATA(pFrameArray), cols, rows, QImage::Format_BGR888);

			//std::cout << "! 3" << std::endl;
			
			// 在 VideoWidget 中显示视频帧
			videoWidget.setImage(image);

			//std::cout << "! 4" << std::endl;

			now = std::chrono::system_clock::now();
			nowNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
			epochNs = nowNs.time_since_epoch();
			timeNs = epochNs.count();
			long long timeNsPY = PyLong_AsLongLong(pTimeNs);
			if (timeNsPY < 0 && PyErr_Occurred()) {
				PyErr_Print();
				// TODO 还不知道怎么处理
			}
			totalLatency += timeNs - timeNsPY;
			framePassed += 1;
			if (framePassed % fps == 0) {
				std::cout << "[Latency (ns)] Total: " << totalLatency << " \tAverage: " << totalLatency / framePassed << " \tframePassed: " << framePassed << std::endl;
			}

			duration = now - startTime;
			long long playedTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
			std::this_thread::sleep_for(std::chrono::milliseconds(playedTimeMs < fileTimeMs ? fileTimeMs - playedTimeMs : 1));

			//std::cout << "! 5" << std::endl;

			//QThread::msleep(playedTimeMS < fileTimeMS ? fileTimeMS - playedTimeMS : 1);

			//std::cout << "! 6" << std::endl;
		}
		Py_END_ALLOW_THREADS
		});
	
	// 在 Qt 槽中连接 QTimer 的超时信号
	//QObject::connect(&timer, &QTimer::timeout, [&]() {
	//	if (pRetVal != nullptr) {
	//		Py_DECREF(pRetVal);
	//	}
	//	//PyObject *pRetVal = PyObject_CallMethod(pCap, "read", nullptr);
	//	pRetVal = PyObject_CallObject(pReadVideo, nullptr);
	//	PyObject* pIsRead = PyTuple_GetItem(pRetVal, 0);
	//	PyObject* pFrame = PyTuple_GetItem(pRetVal, 1);
	//	PyObject* pTimeNs = PyTuple_GetItem(pRetVal, 3);

	//	if (pIsRead == Py_False) {
	//		timer.stop();  // 视频播放结束
	//		Py_DECREF(pRetVal);
	//		now = std::chrono::system_clock::now();
	//		auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
	//		long long seconds = duration.count();
	//		std::cout << "Video Playback Time: " << seconds / 60 << "m" << seconds % 60 << "s (" << seconds << "s)" << std::endl;
	//		return;
	//	}

	//	// 将 Python 的 numpy.array 转换为 QImage
	//	PyArrayObject* pFrameArray = reinterpret_cast<PyArrayObject*>(pFrame);
	//	int rows = PyArray_SHAPE(pFrameArray)[0];
	//	int cols = PyArray_SHAPE(pFrameArray)[1];
	//	int channels = PyArray_SHAPE(pFrameArray)[2];

	//	QImage image((uchar*)PyArray_DATA(pFrameArray), cols, rows, QImage::Format_BGR888);

	//	// 在 VideoWidget 中显示视频帧
	//	videoWidget.setImage(image);

	//	now = std::chrono::system_clock::now();
	//	nowNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
	//	epochNs = nowNs.time_since_epoch();
	//	timeNs = epochNs.count();
	//	long long timeNsPY = PyLong_AsLongLong(pTimeNs);
	//	if (timeNsPY < 0 && PyErr_Occurred()) {
	//		PyErr_Print();
	//		// TODO 还不知道怎么处理
	//	}
	//	totalLatency += timeNs - timeNsPY;
	//	framePassed += 1;
	//	if (framePassed % fps == 0) {
	//		std::cout << "[Latency (ns)] Total: " << totalLatency << " \tAverage: " << totalLatency / framePassed << " \tframePassed: " << framePassed << std::endl;
	//	}
	//	});

	//timer.start();

	//video_thread->start();

	// 运行 Qt 应用
	int ret = app->exec();
	
	//video_thread->wait();
	video_thread.join();
	//audio_thread.join();

	std::cout << "Releasing" << std::endl;
	PyObject_CallObject(pReleaseCapture, nullptr);
	PyObject_CallObject(pReleaseAudio, nullptr);

	// 释放 Python 对象
	Py_DECREF(pModule);
	//Py_DECREF(pFunc);
	Py_DECREF(pInitCapture);
	Py_DECREF(pReadVideo);
	Py_DECREF(pVideoSize);
	Py_DECREF(pReleaseCapture);
	Py_DECREF(pFrameInfo);
	Py_DECREF(pVideoPath);
	Py_DECREF(pCap);
	Py_DECREF(pSize);
	Py_DECREF(pFPS);
	//Py_DECREF(pIsOpened);

	Py_DECREF(pAudioModule);
	Py_DECREF(pInitAudio);
	Py_DECREF(pAudioSamples);
	Py_DECREF(pReadAudio);
	Py_DECREF(pAudioFinished);
	Py_DECREF(pReleaseAudio);
	Py_DECREF(pAudioPath);
	Py_DECREF(pAudioFile);
	Py_DECREF(pAudioInfo);

	Py_Finalize();

	return ret;
}


