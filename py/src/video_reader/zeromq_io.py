
import asyncio
import time
import sys

import zmq

from video_reader.zeromq import ServerManager, MessageServer, DataServer, ValueSlot, ValueName, ServerStatus

audio_queue = asyncio.Queue()

class AudioInputServer(DataServer):
    def __init__(self):
        super().__init__("ipc://temp/audio_input_publish.ipc", zmq.SUB)

    @property
    def name(self):
        return "Audio Input Server"
    
    async def init(self):
        print(f"[{self.name}] Initializing...")
        self.socket.connect(self.endpoint)
        self.socket.setsockopt(zmq.SUBSCRIBE, b'')
        print(f"[{self.name}] Ready!", self.endpoint)

    async def ask_for_values(self):
        print(f"[{self.name}] Asking for Ready")
        self.audio_path:str = await ValueSlot.ask_for(ValueName.audio_ready)
        ServerManager().status = ServerStatus.Playing
        print(f"[{self.name}] Ready to Receive")

    async def deal_data(self):
        print(f"[{self.name}] Start Receiving")
        current_seconds = 0
        start_time = time.time()
        latency = 0
        read_times = 0
        try:
            while not ServerManager()._stop_event.is_set():
                data:list[bytes] = await self.socket.recv_multipart()
                samples, time_bytes = data
                await audio_queue.put(data)
                print(f"[{self.name}] Received {len(samples) // 4} samples")
                send_ns = int.from_bytes(time_bytes, "little")
                latency += time.time_ns() - send_ns
                read_times += 1
                if (time.time() - start_time) > current_seconds:
                    current_seconds += 1
                    print(f"[Latency (ns)] Input: {latency} \tAverage: {latency // read_times} \tread_times: {read_times}")
        except asyncio.TimeoutError:
            print(f"[{self.name}] Timeout")

class AudioOutputServer(DataServer):
    def __init__(self):
        super().__init__("ipc://temp/audio_publish.ipc")

    @property
    def name(self):
        return "Audio Output Server"
    
    async def ask_for_values(self):
        print(f"[{self.name}] Ready to Send")
    
    async def deal_data(self):
        print(f"[{self.name}] Start Sending")
        try:
            while not ServerManager()._stop_event.is_set():
                data_tuple = await audio_queue.get()
                print(f"[{self.name}] Sending...")
                await self.socket.send_multipart(data_tuple, copy=False)
        except asyncio.TimeoutError:
            print(f"[{self.name}] Timeout")

ServerManager().servers = [MessageServer, AudioInputServer, AudioOutputServer]

main = ServerManager().run

if __name__ == "__main__":

    import platform
    if platform.system() == "Windows":
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    
    asyncio.run(main())
