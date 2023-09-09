from __future__ import annotations

import sounddevice as sd
import soundfile as sf
import time

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    import numpy as np

audio_file = None

def init_audio(audio_path):
    global audio_file
    audio_file = sf.SoundFile(audio_path)
    return audio_file

def audio_finished():
    # print(audio_file.tell(), "/", audio_file.frames)
    return audio_file.tell() >= audio_file.frames

def read_audio(samples:int):
    return audio_file.read(samples), time.time_ns()

def read_audio_int16(samples:int):
    return audio_file.read(samples, dtype='int16'), time.time_ns()

def read_audio_float32(samples:int):
    return audio_file.read(samples, dtype='float32'), time.time_ns()

def read_audio_bytes(samples:int):
    # print("samples", samples)
    return audio_file.read(samples, dtype='int16').tobytes(), time.time_ns().to_bytes(8, "big") # long long 8 bytes

def audio_samples() -> tuple[int, int, int]:
    return audio_file.samplerate, audio_file.frames, audio_file.channels

def release_audio():
    audio_file.close()

if __name__ == "__main__":
    path = "../resource/audio.mp3"
    audio_file = init_audio(path)
    sr, samples, channels = audio_samples()
    # print(audio_file.samplerate)    # 48000
    # print(audio_file.frames)        # 10190848
    # print(audio_file.channels)      # 2
    # print(audio_file.format)        # MP3
    # print(audio_file.format_info)   # MPEG-1/2 Audio
    # print(audio_file.endian)        # FILE
    # print(audio_file.mode)          # r
    # print(audio_file.sections)      # 1
    # print(audio_file.subtype)       # MPEG_LAYER_III
    # print(audio_file.subtype_info)  # MPEG Layer III
    # print(audio_file.extra_info)

    def callback(outdata:np.ndarray, frames:int, time_, status):
        samples, time_ns = read_audio( frames )
        # print(frames)
        # outdata.shape = samples.shape
        outdata[:] = samples

    with sd.OutputStream(sr, channels=channels, blocksize=0, callback=callback) as stream:
        while not audio_finished():
            pass

    release_audio()
