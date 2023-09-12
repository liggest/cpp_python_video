
import zmq
from zmq.asyncio import Context, Socket

import sounddevice as sd

import asyncio
import time

from pathlib import Path
Path("temp").mkdir(555,exist_ok=True)

values = {
    "AUDIO_PATH": "../resource/audio.mp3",
}

async def message_client():
    m_context = Context()
    m_socket = m_context.socket(zmq.REQ)

    m_socket.connect("ipc://temp/message.ipc")

    sr, samples, channels = None, None, None
    audio_task = None
    ready = asyncio.Event()

    await m_socket.send_string("STATUS")
    while message := await m_socket.recv_string():
        print(message)
        if message == "IDLE":
            await m_socket.send_string("STATUS")
            continue
        if message == "PLAYING":
            if not sr:
                await m_socket.send_string(f'GET AUDIO_INFO')   
            elif not audio_task:
                audio_task = asyncio.create_task(audio_client(sr, samples, channels, ready))
                await ready.wait()
                await m_socket.send_string("SET AUDIO_READY TRUE")
            else:
                await audio_task
                await m_socket.send_string(f'SET END TRUE')
                break
            continue
        if message.startswith("GET"):
            res = []
            for name in message.removeprefix("GET ").split():
                if value := values.get(name):
                    print(name, ":", value)
                    res.append(f"{name} {value}")
            await m_socket.send_string(f'SET {" ".join(res)}')
            continue
        if message.startswith("SET"):
            name_values = message.removeprefix("SET ").split()
            for name, value in zip(name_values[::2], name_values[1::2]):
                if name == "AUDIO_INFO":
                    sr, samples, channels = [int(num) for num in value.split("|")]
            await m_socket.send_string("STATUS")
            continue
    print("Client End")

import numpy as np
from collections import deque

buffer = deque()

def read_audio():
    if buffer:
        return buffer.popleft()
    
def callback(outdata:np.ndarray, frames:int, time_, status):
    data = read_audio()
    if data is not None:
        outdata[:] = data
    else:
        outdata.fill(0)

async def audio_client(sr:int, total_samples:int, channels:int, ready:asyncio.Event):
    print("audio_client", sr, total_samples, channels)
    a_context = Context()
    a_socket = a_context.socket(zmq.SUB)
    a_socket.connect("ipc://temp/audio_publish.ipc")
    a_socket.setsockopt(zmq.SUBSCRIBE, b'')
    current_seconds = 0
    latency = 0
    read_times = 0
    with sd.OutputStream(sr, channels=channels, blocksize=1024, callback=callback, dtype="int16") as stream:
        values["AUDIO_READY"] = "TRUE"
        ready.set()
        print("wait for recv_multipart")
        try:
            while datas := await asyncio.wait_for(a_socket.recv_multipart(), timeout=5):
                buffer.append(np.frombuffer(datas[0], dtype="int16").reshape((-1, 2)))
                # print(int.from_bytes(datas[1], "little"))
                send_ns = int.from_bytes(datas[1], "little")
                latency += time.time_ns() - send_ns
                read_times += 1
                if read_times * 1024 // sr > current_seconds:
                    print(f"[Latency (ns)] Communication: {latency} \tAverage: {latency // read_times} \tread_times: {read_times}")
                    current_seconds += 1
        except asyncio.TimeoutError:
            print("Read Timeout")
        while buffer:
            print(len(buffer))
            await asyncio.sleep(1)
            


import platform
if platform.system() == "Windows":
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

asyncio.run(message_client())

