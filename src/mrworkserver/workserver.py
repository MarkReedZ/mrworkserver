
import socket, os, asyncio, time
from concurrent.futures import ProcessPoolExecutor
from io import BytesIO

from mrworkserver import Protocol, CWorkServer
from mrworkserver import setup_statserver, stats_timer


class WorkServer(CWorkServer):
  def __init__(self, *, protocol_factory=None, seconds_to_gather=0, collect_stats=False, callback=None):
    self._loop = asyncio.new_event_loop()
    asyncio.set_event_loop(self.loop)
    self._connections = set()
    self._protocol_factory = protocol_factory or Protocol
    self._seconds_to_gather = seconds_to_gather
    self.cb = callback
    self.async_times = None
    self.collect_stats = False
    self.async_times_1m = []
    self.stats_task = None
    self.webserver_task = None
    if collect_stats:
      self.collect_stats = True
      self.cdata = {}
      
      self.async_times = []
      self.mem = []
      self.cpu = []
      self.counts = {}
      self.procpool = ProcessPoolExecutor(2)


      self.webserver_task = setup_statserver(self)

      #app = Sanic(__name__,configure_logging=False)
      #@app.route("/<name>")
      #async def test(r,name):
        #if name == "time": 
          #data = self.async_times
          #title = 'Processing time (ms)'
        #elif name == "cpu": 
          #data = self.cpu
          #title = 'CPU Utilization'
        #elif name == "mem": 
          #data = self.mem
          #title = 'Memory Usage'
        #elif name in self.counts:
          #data = self.counts[name]["cnts"]
          #title = self.counts[name]["title"]
        #else:
          #raise NotFound("OK")
        #b = await self.loop.run_in_executor( self.procpool, blocks, title, data )
        #return response.raw(b,headers={'Content-Type': 'image/svg+xml'})
#
      #server = app.create_server(host="0.0.0.0", port=5000)
      #self.webserver_task = asyncio.ensure_future(server)



    self.on_start = None
    self.on_stop  = None
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


  # Stats
  def count_title(self, name, title):
    if not name in self.counts:
      self.counts[name] = { "title":title, "cnt":0, "cnts":[] }
    self.counts[name]["title"] = title

  def count(self, name, cnt):
    if not name in self.counts:
      self.counts[name] = { "title":name, "cnt":0, "cnts":[] }
    self.counts[name]["cnt"] += cnt 


  def run(self, host='0.0.0.0', port=7100, *, cores=None, ssl=None):

    if not asyncio.iscoroutinefunction(self.cb):
      print("WorkServer.cb must be an async function")
      return;

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((host, port))
    os.set_inheritable(sock.fileno(), True)

    loop = self.loop
    #asyncio.set_event_loop(loop)
    if self.on_start:
      loop.run_until_complete( self.on_start(self) )

    if self.collect_stats: 
      self.stats_task = asyncio.ensure_future( stats_timer(self) )

    server_coro = loop.create_server( lambda: self._protocol_factory(self), sock=sock, ssl=ssl)
    server = loop.run_until_complete(server_coro)

    try:
      loop.run_forever()
    except KeyboardInterrupt:
      pass
    finally:
      if self.stats_task: self.stats_task.cancel()
      if self.webserver_task: self.webserver_task.cancel()
      server.close()
      loop = asyncio.get_event_loop()
      if self.on_stop:
        loop.run_until_complete( self.on_stop(self) )
      loop.run_until_complete(server.wait_closed())
      loop.close()

#class ChartData:
  #title = ""
  #data = []
  

def blocks(title, data):
  try:
    line_chart = pygal.Line()
    line_chart.title = title
    line_chart.add('avg', data)
    return line_chart.render()
  except Exception as e:
    print("chart exception", args, str(e))
    return b''
    

  #b = BytesIO()
  #plt.plot(data)
  #plt.savefig(b, format='png')
  #return b.getvalue()
  #plt.savefig('foo.png', bbox_inches='tight') 


