import difflib
import asyncio, ssl, sys, random
import mrworkserver, mrpacker


num = 0
users = {}

async def on_start(ws):
  print("on_start")
  ws.gather = asyncio.ensure_future( gather(ws) )

async def on_stop(ws):
  print("on_stop, num =",num)

async def callback(ws, msgs):
  for m in msgs:
    num += 1
  print ("Processing",num)

def setcb(ws, k, v):
  users[k] = mrpacker.pack(v)

def fetchcb(ws, o):
  return mrpacker.pack( {"name":"mark"} )

async def gather(ws):
  while True:
    await asyncio.sleep(5)
    ws.process_messages()

# In gather mode you must periodically call ws.process_messages to gather the collected messages
#   If set to False your callback will be called immediately as messages are received

ws = mrworkserver.WorkServer(gather=True,callback=callback,fetch_callback=fetchcb)
ws.setcb = setcb
ws.on_start = on_start
ws.on_stop = on_stop

port = 7100
print (sys.argv)
if len(sys.argv) == 2:
  port = int(sys.argv[1])

#sc = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
#sc.load_cert_chain(certfile='cert/server.crt', keyfile='cert/server.key')
ws.run(host="127.0.0.1",port=port) #ssl=sc)

