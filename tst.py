
import asyncio, ssl, sys, random
import mrworkserver, mrpacker

num = 0
async def on_start(ws):
  print("on_start")
async def on_stop(ws):
  global num
  print("on_stop")
  print( "num",num)

async def callback(ws, msgs):
  global num
  #print(msgs)
  #await asyncio.sleep( random.uniform( 0.01, 0.1 ) )
  for m in msgs:
    num += 1
    #print(m)

def fetchcb(ws, o):
  #return "notbytes"
  return mrpacker.pack([1,2,3])

cs = True
ws = mrworkserver.WorkServer(seconds_to_gather=5,callback=callback,collect_stats=cs,fetch_callback=fetchcb)
ws.on_start = on_start
ws.on_stop = on_stop
import time

port = 7100
print (sys.argv)
if len(sys.argv) == 2:
  port = int(sys.argv[1])

#sc = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
#sc.load_cert_chain(certfile='cert/server.crt', keyfile='cert/server.key')
ws.run(host="127.0.0.1",port=port) #ssl=sc)

