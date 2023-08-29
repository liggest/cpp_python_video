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
    return *cap.read(), time.time_ns()  # retval, image，time

def video_size():
    return cap.get(cv2.CAP_PROP_FRAME_WIDTH), cap.get(cv2.CAP_PROP_FRAME_HEIGHT)

def video_frames():
    return cap.get(cv2.CAP_PROP_FRAME_COUNT), cap.get(cv2.CAP_PROP_FPS)
