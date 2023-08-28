import cv2
import sys
import time

def read_video(video_path):
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print("Cannot open the video file")
        sys.exit()
    return cap
