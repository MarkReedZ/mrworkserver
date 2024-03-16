
import mrworkserver

cac = {}

async def on_start(ws):
  print("on_start - setup your DB connections here")
async def on_stop(ws):
  print("on_stop - cleanup here")

# MrWorkserver has get and set 
def setcb(ws, k, v):
  global cac
  cac[k] = v

def fetchcb(ws, k):
  if k in cac:
    return cac[k]
  return None

async def callback(ws, msgs):
  global num
  for m in msgs:
    num += 1
  print (f"Processed {num} messages")

ws = mrworkserver.WorkServer(callback=callback,fetch_callback=fetchcb)
ws.setcb = setcb
ws.on_start = on_start
ws.on_stop = on_stop

ws.run(host="127.0.0.1",port=7100)

