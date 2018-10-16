from sanic import Sanic
from sanic import response
from sanic.exceptions import NotFound
import asyncio
import pygal
import psutil

import mrworkserver
from mrhttp import escape
import tenjin, os
from tenjin.helpers import *
tenjin.set_template_encoding('utf-8')
engine = tenjin.Engine(path=[os.path.dirname(__file__)+'/html'],escapefunc='escape',tostrfunc='str')

from pygal.style import DarkStyle
cstyle = DarkStyle(
  title_font_size=28,
  label_font_size=24,
  major_label_font_size=24
)

def setup_statserver(ws):

  app = Sanic(__name__,configure_logging=False)
  @app.route("/<name>")
  async def test(r,name):
    if name == "time":
      data = ws.async_times
      title = 'Processing time (ms)'
    elif name == "cpu":
      data = ws.cpu
      title = 'CPU Utilization'
    elif name == "mem":
      data = ws.mem
      title = 'Memory Usage'
    elif name in ws.counts:
      data = ws.counts[name]["cnts"]
      title = ws.counts[name]["title"]
    else:
      raise NotFound("OK")
    b = await ws.loop.run_in_executor( ws.procpool, blocks, title, data )
    return response.raw(b,headers={'Content-Type': 'image/svg+xml'})

  @app.route("/")
  async def index(r):
    imgs = ["time","cpu","mem"]
    imgs.extend( ws.counts.keys() )
    return response.html(engine.render('stats.ten', {"imgs":imgs}))

  server = app.create_server(host="0.0.0.0", port=5000)
  return asyncio.ensure_future(server)

async def stats_timer(self):
  minute = 0
  while 1:
    await asyncio.sleep(60)
    self.mem.append( psutil.virtual_memory().percent )
    self.cpu.append( psutil.cpu_percent() )

    for k in self.counts:
      c = self.counts[k]
      c["cnts"].append( c["cnt"] )
      c["cnt"] = 0

    if len( self.async_times ) > 0:
      tot = 0
      num = 0
      for ms in self.async_times:
        tot += ms
        num += 1
      avg = tot/num
      self.async_times_1m.append(avg)
      #self.async_times = []
    minute += 1
    if minute == 1440:
      # TODO Store 10m times to disk
      pass


def blocks(title, data):
  try:
    line_chart = pygal.Line(style=cstyle,show_legend=False)
    line_chart.title = title
    line_chart.add('', data)
    return line_chart.render()
  except Exception as e:
    print("chart exception", args, str(e))
    return b''

