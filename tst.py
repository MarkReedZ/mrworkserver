
import mrworkserver

num = 0

async def callback(ws, msgs):
  global num
  for m in msgs:
    num += 1
  print ("Saw",num,"messages")

ws = mrworkserver.WorkServer(callback=callback)

ws.run(host="127.0.0.1",port=7100)

