
import socket, os, asyncio

from mrworkserver import Protocol

class WorkServer():
  def __init__(self, *, protocol_factory=None):
    self._loop = None
    self._connections = set()
    self._protocol_factory = protocol_factory or Protocol
    self.cb = None
    self.workq = asyncio.Queue(loop=self.loop)


  @property
  def loop(self):
    if not self._loop:
      self._loop = asyncio.new_event_loop()
    return self._loop

  def run(self, host='0.0.0.0', port=7100, *, cores=None, ssl=None):

    if not asyncio.iscoroutinefunction(self.cb):
      print("WorkServer.cb must be an async function")
      return;

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((host, port))
    os.set_inheritable(sock.fileno(), True)

    loop = self.loop
    asyncio.set_event_loop(loop)
    server_coro = loop.create_server( lambda: self._protocol_factory(self), sock=sock, ssl=ssl)
    server = loop.run_until_complete(server_coro)

    try:
      loop.run_forever()
    except KeyboardInterrupt:
      pass
    finally:
      server.close()
      loop = asyncio.get_event_loop()
      loop.run_until_complete(server.wait_closed())
      loop.close()

