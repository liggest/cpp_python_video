import cv2
# import sys
import time

cap = None

# def printOut(obj):
#     print(obj)
#     return obj

def init_capture(video_path):
    global cap
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print("Cannot open the video file")
        cap = None
    return cap

def read_video():
    return *cap.read(), cap.get(cv2.CAP_PROP_POS_MSEC), time.time_ns()  # retval, image，file_time_ms, time

def video_size():
    return cap.get(cv2.CAP_PROP_FRAME_WIDTH), cap.get(cv2.CAP_PROP_FRAME_HEIGHT)

def video_frames():
    return cap.get(cv2.CAP_PROP_FRAME_COUNT), cap.get(cv2.CAP_PROP_FPS)

def release_capture():
    cap.release()

if __name__ == "__main__":
    path = "../resource/video.mp4"
    init_capture(path)

    frames, fps = video_frames()
    interval = int(1000 / fps)
    start_time_ns = time.time_ns()
    ms2ns = 10**6
    # 循环读取并显示视频帧
    while True:
        # 读取一帧视频
        ret, frame, file_time_ms ,time_ns = read_video()
        interval = max(1, int(file_time_ms - (time_ns - start_time_ns) / ms2ns))
        # print(interval)
        # 如果读取失败，说明视频结束
        if not ret:
            break

        # 显示视频帧
        cv2.imshow("Video", frame)

        # 设置每帧的延迟时间为 interval
        key = cv2.waitKey(interval)

        # 如果按下 c 键，退出循环
        if key == ord("c"):
            break

    # 释放 VideoCapture 对象和窗口
    release_capture()
    cv2.destroyAllWindows()
