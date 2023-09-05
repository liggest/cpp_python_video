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
    audio_file.tell() >= audio_file.frames

def read_audio(samples:int):
    return audio_file.read(samples), time.time_ns()

def audio_samples():
    return audio_file.samplerate, audio_file.frames, audio_file.channels

def release_audio():
    audio_file.close()

if __name__ == "__main__":
    path = "../resource/audio.mp3"
    init_audio(path)
    sr, samples, channels = audio_samples()

    def callback(outdata:np.ndarray, frames:int, time_, status):
        samples, time_ns = read_audio( frames )
        # print(frames)
        # outdata.shape = samples.shape
        outdata[:] = samples

    with sd.OutputStream(sr, channels=channels, blocksize=0, callback=callback) as stream:
        while not audio_finished():
            pass

    release_audio()
