



def escape(x):
  return x
  

def setup_statserver(ws):
  global pygal, cstyle, escape
  try: 
    import asyncio
    from mrhttp import app
    import pygal
    import psutil
    
    #import mrworkserver
    from mrhttp import escape
    import tenjin, os
    #from tenjin.helpers import *
    tenjin.set_template_encoding('utf-8')
    engine = tenjin.Engine(path=[os.path.dirname(__file__)+'/html'],escapefunc='escape',tostrfunc='str')
    from pygal.style import DarkStyle
    cstyle = DarkStyle(
      title_font_size=28,
      label_font_size=24,
      major_label_font_size=24
    )
  except Exception as e:
    print("Enabling stats collection requires the following modules which appear to be missing:")
    print("pip install pygal psutil tenjin mrhttp")
    print(e)
    exit(0)


  @app.route('/{}')
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
      raise r.NotFound()
    b = await ws.loop.run_in_executor( ws.procpool, blocks, title, data )
   
    r.response.headers["Content-Type"] = 'image/svg+xml'
    return b.decode("utf-8")

  @app.route('/')
  async def index(r):
    imgs = ["time","cpu","mem"]
    imgs.extend( ws.counts.keys() )
    return engine.render('stats.ten', {"imgs":imgs})

  try:
    server = app.start_server(host="0.0.0.0", port=7099, loop=ws.loop)
  except Exception as e:
    print(e)
  return asyncio.ensure_future(server, loop=ws.loop)

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
    print("chart exception", title, data, str(e))
    return b''

