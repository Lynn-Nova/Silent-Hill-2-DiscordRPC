import ctypes
import json
import pathlib
import struct
import subprocess
import sys
import threading
import time
import unittest
from ctypes import wintypes


CLIENT = str(pathlib.Path(sys.argv.pop(1)).resolve())
KERNEL = ctypes.WinDLL("kernel32", use_last_error=True)
KERNEL.CreateNamedPipeW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, wintypes.DWORD, wintypes.DWORD, wintypes.DWORD, wintypes.DWORD, ctypes.c_void_p]
KERNEL.CreateNamedPipeW.restype = wintypes.HANDLE
KERNEL.ConnectNamedPipe.argtypes = [wintypes.HANDLE, ctypes.c_void_p]
KERNEL.ReadFile.argtypes = [wintypes.HANDLE, ctypes.c_void_p, wintypes.DWORD, ctypes.POINTER(wintypes.DWORD), ctypes.c_void_p]
KERNEL.WriteFile.argtypes = KERNEL.ReadFile.argtypes
KERNEL.CloseHandle.argtypes = [wintypes.HANDLE]
INVALID_HANDLE = ctypes.c_void_p(-1).value


class FakeDiscord:
    def __init__(self):
        self.handle = KERNEL.CreateNamedPipeW(r"\\.\pipe\discord-ipc-9", 3 | 0x00080000, 1, 1, 65536, 65536, 0, None)
        if self.handle == INVALID_HANDLE:
            raise ctypes.WinError(ctypes.get_last_error())

    def close(self):
        if self.handle is not None:
            KERNEL.CloseHandle(self.handle)
            self.handle = None

    def accept(self):
        deadline = time.monotonic() + 8
        while time.monotonic() < deadline:
            if KERNEL.ConnectNamedPipe(self.handle, None):
                return
            error = ctypes.get_last_error()
            if error == 535:
                return
            if error not in (536, 232):
                raise ctypes.WinError(error)
            time.sleep(0.01)
        raise TimeoutError("Client did not connect.")

    def read_exact(self, length):
        output = bytearray()
        deadline = time.monotonic() + 8
        while len(output) < length and time.monotonic() < deadline:
            buffer = ctypes.create_string_buffer(length - len(output))
            count = wintypes.DWORD()
            success = KERNEL.ReadFile(self.handle, buffer, len(buffer), ctypes.byref(count), None)
            if success and count.value:
                output.extend(buffer.raw[:count.value])
            elif not success and ctypes.get_last_error() != 232:
                raise ctypes.WinError(ctypes.get_last_error())
            else:
                time.sleep(0.01)
        if len(output) != length:
            raise TimeoutError("Client frame incomplete.")
        return bytes(output)

    def read_frame(self):
        opcode, length = struct.unpack("<II", self.read_exact(8))
        return opcode, self.read_exact(length)

    def write(self, data):
        buffer = ctypes.create_string_buffer(data)
        count = wintypes.DWORD()
        if not KERNEL.WriteFile(self.handle, buffer, len(data), ctypes.byref(count), None) or count.value != len(data):
            raise ctypes.WinError(ctypes.get_last_error())

    def send_frame(self, opcode, payload, fragmented=False):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        frame = struct.pack("<II", opcode, len(body)) + body
        if fragmented:
            for chunk in (frame[:3], frame[3:8], frame[8:13], frame[13:]):
                self.write(chunk)
                time.sleep(0.025)
        else:
            self.write(frame)


class IpcTests(unittest.TestCase):
    def run_exchange(self, exchange, failure=False):
        server = FakeDiscord()
        errors = []

        def serve():
            try:
                server.accept()
                opcode, handshake = server.read_frame()
                self.assertEqual(opcode, 0)
                self.assertEqual(json.loads(handshake)["client_id"], "1560708389796061245")
                exchange(server)
            except BaseException as error:
                errors.append(error)
            finally:
                server.close()

        thread = threading.Thread(target=serve, daemon=True)
        thread.start()
        try:
            result = subprocess.run([CLIENT] + (["failure"] if failure else []), capture_output=True, text=True, timeout=15)
            thread.join(timeout=10)
            self.assertFalse(thread.is_alive(), "Fake Discord did not finish.")
            if errors:
                raise errors[0]
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        finally:
            server.close()

    def test_partial_frames_ping_unicode_nonce_and_cleanup(self):
        def exchange(server):
            server.send_frame(1, {"evt": "READY", "data": {}}, fragmented=True)
            opcode, raw = server.read_frame()
            self.assertEqual(opcode, 1)
            request = json.loads(raw)
            activity = request["args"]["activity"]
            self.assertEqual(activity["details"], "Explorando Wood Side — Outro Mundo")
            self.assertEqual(activity["timestamps"]["start"], 1700000000)
            self.assertGreater(request["args"]["pid"], 0)
            server.send_frame(3, {"ping": "probe"})
            opcode, pong = server.read_frame()
            self.assertEqual(opcode, 4)
            self.assertEqual(json.loads(pong), {"ping": "probe"})
            server.send_frame(1, {"evt": None, "cmd": "SET_ACTIVITY", "nonce": "unrelated"})
            server.send_frame(1, {"evt": None, "cmd": "SET_ACTIVITY", "nonce": request["nonce"], "data": activity}, fragmented=True)
            opcode, clear = server.read_frame()
            self.assertEqual(opcode, 1)
            self.assertIsNone(json.loads(clear)["args"]["activity"])
        self.run_exchange(exchange)

    def test_oversized_frame_is_rejected(self):
        self.run_exchange(lambda server: server.write(struct.pack("<II", 1, 262145)), failure=True)

    def test_discord_error_is_rejected(self):
        self.run_exchange(lambda server: server.send_frame(1, {"evt": "ERROR", "data": {"message": "Invalid client ID"}}), failure=True)

    def test_disconnect_during_header_is_handled(self):
        self.run_exchange(lambda server: server.write(struct.pack("<I", 1)), failure=True)

    def test_unresponsive_discord_times_out(self):
        self.run_exchange(lambda server: time.sleep(5.5), failure=True)


if __name__ == "__main__":
    unittest.main(verbosity=2)
