from __future__ import annotations
from typing import Callable, Protocol
from typing_extensions import Self

import time
import asyncio
from enum import Enum

import zmq
from zmq.asyncio import Context
from zmq.utils.monitor import recv_monitor_message, _MonitorMessage

from video_reader.util import SingletonMeta
from video_reader.audio import init_audio, audio_samples, read_audio_bytes, audio_finished, release_audio

from pathlib import Path
Path("temp").mkdir(555,exist_ok=True)

class Messages(str, Enum):
    # hello = "HELLO"
    # ok = "OK"
    # ready = "READY"
    # read = "READ"
    status = "STATUS"
    """ 客户端询问服务端状态 """
    get = "GET"
    """ 需要值 """
    set = "SET"
    """ 设置值 """
    idle = "IDLE"
    """ 空闲 """
    playing = "PLAYING"
    """ 正在播放 """

class ValueName(str, Enum):
    audio_path = "AUDIO_PATH"
    audio_info = "AUDIO_INFO"
    audio_ready = "AUDIO_READY"
    end = "END"
    video_path = "VIDEO_PATH"
    video_info = "VIDEO_INFO"

class ValueSlot:

    _value_required:dict[ValueName, ValueSlot] = {}
    inited = False

    def __new__(cls, name):
        if obj := cls._value_required.get(name):
            return obj
        return super().__new__(cls)

    def __init__(self, name:ValueName):
        if not self.inited:
            print("Add Slot:", name)
            self.name = name
            self._future = asyncio.Future()
            self._value_required[name] = self
            if ServerManager().status.value < ServerStatus.NeedValue.value:
                ServerManager().status = ServerStatus.NeedValue
            self.inited = True

    @classmethod
    async def ask_for(cls, name:ValueName):
        return await cls(name).get()

    async def get(self):
        if not self.had_value():
            await self._future
        return self._future.result()

    def set(self, value):
        self._future.set_result(value)

    def had_value(self):
        return self._future.done()

class MessageServer:

    LISTEN_ALL_EVENTS = -1
    event_names:dict[int, str] = { getattr(zmq, name):name for name in dir(zmq) if name.startswith("EVENT_") }
    event_names[LISTEN_ALL_EVENTS] = "LISTEN_ALL_EVENTS"
    # event_id : name

    def __init__(self):
        self.context = Context()
        self.socket = self.context.socket(zmq.REP)  # 接收消息
        self._on_socket_evnets:dict[int, Callable[[_MonitorMessage], None]] = {}

    async def init(self):
        print("[Message Server] Initializing...")
        self.socket.bind("ipc://temp/message.ipc")
        print("[Message Server] Ready!")

    async def listen(self):
        print("[Message Server] Start Listening...")
        try:
            while not ServerManager()._stop_event.is_set():
                message = await self.socket.recv_string()
                await self.handle_message(message)
        except asyncio.TimeoutError:
            print("[Message Server] Listening Timeout")
        finally:
            print("[Message Server] Listening Stopped")

    async def life_cycle(self):
        print("[Message Server] Getting Event Monitor...")
        monitor = self.socket.get_monitor_socket()
        try:
            while await monitor.poll() and not ServerManager()._stop_event.is_set():
                event = await recv_monitor_message(monitor)
                self.emit_event(event)
                if event["event"] == zmq.EVENT_MONITOR_STOPPED:
                    break
        except asyncio.TimeoutError:
            print("[Message Server] Event Monitor Timeout")
        finally:
            monitor.close()
            print("[Message Server] Event Monitor Stopped")

    def emit_event(self, event:_MonitorMessage):
        event_id = event["event"]
        print("[Message Server] Received", self.event_names[event_id])
        if func := self._on_socket_evnets.get(event_id):
            func(event)

    def on_event(self, event_id:int=None):
        if event_id is None:
            event_id = self.LISTEN_ALL_EVENTS
        def inner(func):
            self._on_socket_evnets[event_id] = func
            return func
        return inner

    async def run(self):
        await self.init()
        life_cycle = asyncio.create_task(self.life_cycle())
        listen = asyncio.create_task(self.listen())
        await ServerManager()._stop_event.wait()  # 最好能正常结束
        print("[Message Server] Stopping...")
        print("[Message Server] Ending Tasks...")
        await asyncio.gather( asyncio.wait_for(life_cycle,timeout=3), asyncio.wait_for(listen,timeout=3) )
        print("[Message Server] Server Stopped")

    async def handle_message(self, message:str):
        print("[Message Server] Received Message:", message)
        if not (response := await self.message_response(message)):
            response = self.default_response()  # recv 之后一定要 send
        await self.socket.send_string(response)
        print("[Message Server] Responded:", response)

    async def message_response(self, message:str):
        if message.startswith(Messages.status):
            return self.status_response(self.unprefix(message, Messages.status))
        if message.startswith(Messages.set):
            return self.set_value_response(self.unprefix(message, Messages.set))
        if message.startswith(Messages.get):
            return await self.get_value_response(self.unprefix(message, Messages.get))

    def status_response(self, message:str):
        manager = ServerManager()
        if manager.status == ServerStatus.NeedValue:
            if ValueSlot._value_required:
                # GET {ValueName1} {ValueName2} ...
                return f'{Messages.get} {" ".join(name for name, slot in ValueSlot._value_required.items() if not slot.had_value())}' 
        if manager.status == ServerStatus.Playing:
            return Messages.playing

    def set_value_response(self, message:str):
        values = message.split()
        for name, val in zip(values[::2], values[1::2]):  # {name1} {value1} {name2} {value2}...
            if slot := ValueSlot._value_required.get(name):
                slot.set(val)

    async def get_value_response(self, message:str):
        names = message.split() # {name1} {name2}...
        slots = [ slot for name in names if (slot := ValueSlot._value_required.get(name)) ]
        values = await asyncio.gather(*(slot.get() for slot in slots))
        name_values = " ".join(f"{slot.name} {value}" for slot, value in zip(slots, values))
        return f'{Messages.set} {name_values}'
                
    @staticmethod
    def default_response():
        return Messages.idle
    
    @staticmethod
    def unprefix(message:str, prefix:Messages):
        return message.removeprefix(prefix).strip()

class DataServer:

    def __init__(self, endpoint:str, socket_type=zmq.PUB):  
        self.context = Context()
        self.socket = self.context.socket(socket_type)      # 默认广播数据
        self.endpoint = endpoint

    @property
    def name(self):
        return "Data Server"

    async def init(self):
        print(f"[{self.name}] Initializing...")
        self.socket.bind(self.endpoint)
        print(f"[{self.name}] Ready!", self.endpoint)

    async def run(self):
        await self.init()
        await self.ask_for_values()
        await self.deal_data()
        print(f"[{self.name}] Server Stopped")

    async def ask_for_values(self):
        raise NotImplementedError

    async def deal_data(self):
        raise NotImplementedError

class AudioServer(DataServer):
    
    samplesPreFrame = 1024
    readThreshold = 2048

    def __init__(self):
        super().__init__("ipc://temp/audio_publish.ipc")

    @property
    def name(self):
        return "Audio Server"
    
    async def ask_for_values(self):
        print(f"[{self.name}] Asking for Path")
        self.audio_path:str = await ValueSlot.ask_for(ValueName.audio_path)
        print(f"[{self.name}] Reading Audio Information")
        init_audio(self.audio_path)
        self.sr, self.totalSamples, self.channels = audio_samples()
        ValueSlot(ValueName.audio_info).set(f"{self.sr}|{self.totalSamples}|{self.channels}") # 三个数字一起发送
        ready_task = asyncio.create_task(ValueSlot.ask_for(ValueName.audio_ready))
        ServerManager().status = ServerStatus.Playing
        await ready_task
        print(f"[{self.name}] Ready to Send")

    async def deal_data(self):
        # ServerManager().status = ServerStatus.Playing
        print(f"[{self.name}] Start Sending")
        current_samples = 0
        start_time = time.time()
        sleepTime = (self.samplesPreFrame / self.sr) / 10
        try:
            while not ServerManager()._stop_event.is_set() and not audio_finished():
                current_time = time.time()
                if (current_time - start_time) * self.sr < current_samples - self.readThreshold:
                    await asyncio.sleep(sleepTime)
                    continue
                data_tuple = read_audio_bytes(self.samplesPreFrame)
                current_samples += self.samplesPreFrame
                await self.socket.send_multipart(data_tuple, copy=False)
                # print(int.from_bytes(data_tuple[1], "little"))
                print(f"{current_samples} / {self.totalSamples}")
        except asyncio.TimeoutError:
            print(f"[{self.name}] Timeout")
        finally:
            release_audio()
            print(f"[{self.name}] Released")

class Runnable(Protocol):
    async def run(self):
        ...

class ServerStatus(Enum):
    Idle = 0
    NeedValue = 1
    Playing = 2

class ServerManager(metaclass=SingletonMeta[Self]):

    servers:list[type[Runnable]] = [MessageServer, AudioServer]

    def __init__(self):
        # print("Server Manager Init")
        self.status:ServerStatus = ServerStatus.Idle
        self._stop_event = asyncio.Event()

    async def run(self):
        print("ZeroMQ version", zmq.zmq_version())
        self.register_signal()
        self.server_tasks:dict[Runnable, asyncio.Task] = {
            (server := server_cls()): asyncio.create_task(server.run()) 
            for server_cls in self.servers
        }
        # self.message_server = MessageServer()
        # message_task = asyncio.create_task(self.message_server.run())
        # self.audio_server = AudioServer()
        # audio_task = asyncio.create_task(self.audio_server.run())
        end_task = asyncio.create_task(self.wait_end())
        await self._stop_event.wait()
        print("Stop Event is Set")
        # await asyncio.gather(message_task, audio_task)
        if self.server_tasks:
            await asyncio.gather(*self.server_tasks.values())
        if not end_task.done():
            end_task.cancel()
        print("All Server Stopped")

    async def wait_end(self):
        await ValueSlot.ask_for(ValueName.end)
        self._stop_event.set()

    def register_signal(self):
        import signal
        sigMap={signal.SIGTERM:"SIGTERM", signal.SIGINT:"SIGINT"}
                # signal.CTRL_BREAK_EVENT:"CTRL_BREAK_EVENT", signal.CTRL_C_EVENT:"CTRL_C_EVENT",
                # signal.SIGBREAK:"SIGBREAK"}

        def callback(sig:int,frame=None):
            print(f"检测到 {sigMap[sig]}")
            self._stop_event.set()
        
        try:
            loop=asyncio.get_event_loop()
            for sig in sigMap:
                loop.add_signal_handler(sig, callback, sig)
        except NotImplementedError:
            for sig in sigMap:
                signal.signal(sig,callback)

        import platform
        if platform.system() == "Windows":
            from zmq.utils.win32 import allow_interrupt
            def callback():
                print(f"检测到中断")
                self._stop_event.set()
                    
            allow_interrupt(callback).__enter__() # 感觉像强行掐断了，没触发回调函数

main = ServerManager().run

if __name__ == "__main__":

    import platform
    if platform.system() == "Windows":
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    
    asyncio.run(main())
