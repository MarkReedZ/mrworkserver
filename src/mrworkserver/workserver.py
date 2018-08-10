
import socket, os, asyncio

from mrworkserver import Protocol, CWorkServer

class WorkServer(CWorkServer):
  def __init__(self, *, protocol_factory=None, seconds_to_gather=0, callback=None):
    self._loop = asyncio.new_event_loop()
    self._connections = set()
    self._protocol_factory = protocol_factory or Protocol
    self._seconds_to_gather = seconds_to_gather
    self.cb = callback
    #self.workq = asyncio.Queue(loop=self.loop)
    super(WorkServer,self).__init__(callback, seconds_to_gather);


  @property
  def loop(self):
    if not self._loop:
      self._loop = asyncio.new_event_loop()
    return self._loop

  #def dcConnect(servers):
    #for s in servers:
      #srv = Server( s[0], s[1] )
      #srv.r, srv.w = await asyncio.open_connection( s[0], s[1], loop=self._loop, limit=DEFAULT_BUFFER_SIZE, ssl=ssl)
      #self.servers.append(srv)


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

