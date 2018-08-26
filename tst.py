
import asyncio, ssl, sys
import mrworkserver

async def on_start(ws):
  print("on_start")
async def on_stop(ws):
  print("on_stop")

num = 0
async def callback(ws, msgs):
  global num
  for m in msgs:
    num += 1
    print(m)
  print( "num",num)

ws = mrworkserver.WorkServer(seconds_to_gather=5,callback=callback)
ws.on_start = on_start
ws.on_stop = on_stop

port = 7100
print (sys.argv)
if len(sys.argv) == 2:
  port = int(sys.argv[1])

#sc = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
#sc.load_cert_chain(certfile='cert/server.crt', keyfile='cert/server.key')
ws.run(host="127.0.0.1",port=port) #ssl=sc)

