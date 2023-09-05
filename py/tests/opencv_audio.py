import time

import cv2
import sounddevice as sd

from video_reader.opencv import init_capture, read_video, release_capture
from video_reader.audio import init_audio, read_audio, audio_samples, audio_finished, release_audio

video_path = "../resource/video.mp4"
audio_path = "../resource/audio.mp3"

init_capture(video_path)
init_audio(audio_path)

start_time_ns = time.time_ns()
ms2ns = 10**6

sr, samples, channels = audio_samples()
should_out = sr

# def callback(outdata, frames:int, time_, status):
#     global should_out
#     print(should_out)
#     if should_out >= frames:
#         samples, time_ns = read_audio( frames )
#         # print(frames)
#         # outdata.shape = samples.shape
#         outdata[:] = samples
#         should_out -= frames
#     else:
#         samples, time_ns = read_audio( should_out )
#         outdata[:should_out] = samples
#         outdata[should_out:] = 0
#         should_out = 0

def callback(outdata, frames:int, time_, status):
    samples, time_ns = read_audio( frames )
    outdata[:] = samples

with sd.OutputStream(sr, channels=channels, blocksize=0, callback=callback) as stream:
    # 循环读取并显示视频帧
    last_time_ms = 0
    while True:
        # 读取一帧视频
        ret, frame, file_time_ms ,time_ns = read_video()
        # 如果读取失败，说明视频结束
        if not ret:
            break
        
        should_out += round((file_time_ms - last_time_ms) / 1000 * sr)
        last_time_ms = file_time_ms

        interval = max(1, int(file_time_ms - (time_ns - start_time_ns) / ms2ns))
        # print(interval)

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

release_audio()
