
import asyncio
from enum import Enum

import zmq
from zmq.asyncio import Context, Socket
# import signal

from video_reader.audio import init_audio, audio_samples, read_audio_bytes, audio_finished, release_audio

class Messages(str, Enum):
    hello = "HELLO"
    ok = "OK"
    ready = "READY"
    read = "READ"


context = Context()
socket = context.socket(zmq.PAIR)

async def wait_path(): # get path
    recv_task = (socket.recv_string(), ) # tuple
    while True:
        print("Sending Hello")
        await socket.send_string(Messages.hello)  # "HELLO"
        done, recv_task = await asyncio.wait(recv_task, timeout=1) # 等待 1 秒，如果有结果了会进入 done，否则还是 recv_task
        if done:
            message:str = await tuple(done)[0] # 得到消息
            print("Get Message", message)
            if message.startswith(Messages.ok):         # "OK {path}"
                return message.split(maxsplit=1)[1]

async def send_audio_info(path):
    init_audio(path)
    sr, samples, channels = audio_samples()
    await socket.send_string(f"{Messages.ready} {sr} {samples} {channels}") # "READY {sr} {samples} {channels}"

async def on_init():
    path = await wait_path()
    await send_audio_info(path)

async def send_audio():
    try:
        while (message := await asyncio.wait_for(socket.recv_string(), timeout=5)):  # 5 秒超时
            if message.startswith(Messages.read):
                samples = int(message.split(maxsplit=1)[1])  # "READ {samples}"
                print("Asked to Read", samples, "samples")
                data_tuple = read_audio_bytes(samples)  # data, time_ns (bytes)
                await socket.send_multipart(data_tuple, copy=False)
                if audio_finished():
                    release_audio()
                    print("Audio Finished")
                    break
        # print(repr(message))
    except asyncio.TimeoutError:
        print("Ask for Reading Timeout")

# def registerSignal():
#     sigMap = {signal.SIGTERM:"SIGTERM", signal.SIGINT:"SIGINT"}

#     def callback(sig:int,frame=None):
#         print(f"检测到 {sigMap[sig]}")
#         import sys
#         sys.exit(0)

#     try:
#         loop = asyncio.get_event_loop()
#         for sig in sigMap:
#             loop.add_signal_handler(sig,callback,sig)
#     except NotImplementedError:
#         for sig in sigMap:
#             signal.signal(sig,callback)

async def main():
    # registerSignal()

    print("ZeroMQ version", zmq.zmq_version())
    print("Connecting to the audio server...")
    # socket.connect("ipc://audio_player.ipc")
    socket.connect("ipc://temp/audio_player.ipc")
    # socket.connect("tcp://localhost:5555")

    await on_init()
    print("Ready!")

    print("Sending...")

    await send_audio()

    print("Finished")

if __name__ == "__main__":

    import platform
    if platform.system() == "Windows":
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    
    asyncio.run(main())
