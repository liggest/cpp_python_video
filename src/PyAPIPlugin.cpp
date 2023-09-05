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
	std::string command = "ffplay -vn -nodisp -loglevel quiet -i \"../resource/video.mp4\"";
	std::system(command.c_str());
	
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
		videoWidget.setSize(width, height);
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

	auto now = std::chrono::system_clock::now();
	auto nowNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
	auto epochNs = nowNs.time_since_epoch();
	long long timeNs = epochNs.count();
	auto startTime = now;
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);

	long long fileTimeMS = 0;

	long long totalLatency = 0;
	long framePassed = 0;

	PyObject* pRetVal = nullptr;

	std::thread audio_thread(playAudio);

	//QThread* video_thread = QThread::create([&]() {
	//QTimer::singleShot(0, [&]() {
	std::thread video_thread([&]() {
		std::cout << "QThread Starting" << std::endl;
		Py_BEGIN_ALLOW_THREADS
		PyGILState_STATE gstate = PyGILState_Ensure();  // 获取 GIL
		while(true) {
			if (pRetVal != nullptr) {
				Py_DECREF(pRetVal);
			}
			//std::cout << "! 1" << std::endl;

			pRetVal = PyObject_CallObject(pReadVideo, nullptr);
			
			PyObject* pIsRead = PyTuple_GetItem(pRetVal, 0);

			if (pIsRead == Py_False) {
				Py_DECREF(pRetVal);
				PyGILState_Release(gstate);  // 释放 GIL
				now = std::chrono::system_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
				long long seconds = duration.count();
				std::cout << "Video Playback Time: " << seconds / 60 << "m" << seconds % 60 << "s (" << seconds << "s)" << std::endl;
				long long fileSeconds = fileTimeMS / 1000;
				std::cout << "Video File Duration: " << fileSeconds / 60 << "m" << fileSeconds % 60 << "s (" << fileSeconds << "s)" << std::endl;
				return; // 视频播放结束
			}

			//std::cout << "! 2" << std::endl;

			PyObject* pFrame = PyTuple_GetItem(pRetVal, 1);
			PyObject* pAimedTimeMs = PyTuple_GetItem(pRetVal, 2);
			PyObject* pTimeNs = PyTuple_GetItem(pRetVal, 3);

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

			duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
			fileTimeMS = static_cast<int>(PyFloat_AsDouble(pAimedTimeMs));
			if (fileTimeMS < 0 && PyErr_Occurred()) {
				PyErr_Print();
				// TODO 还不知道怎么处理
			}
			long long playedTimeMS = duration.count();
			std::this_thread::sleep_for(std::chrono::milliseconds(playedTimeMS < fileTimeMS ? fileTimeMS - playedTimeMS : 1));

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
	audio_thread.join();

	PyObject_CallObject(pReleaseCapture, nullptr);

	// 释放 Python 对象
	Py_DECREF(pModule);
	//Py_DECREF(pFunc);
	Py_DECREF(pInitCapture);
	Py_DECREF(pReadVideo);
	Py_DECREF(pVideoSize);
	Py_DECREF(pFrameInfo);
	Py_DECREF(pVideoPath);
	Py_DECREF(pReleaseCapture);
	Py_DECREF(pCap);
	Py_DECREF(pSize);
	Py_DECREF(pFPS);
	//Py_DECREF(pIsOpened);

	Py_Finalize();

	return ret;
}


