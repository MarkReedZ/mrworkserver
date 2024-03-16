
# kill -s SIGHUP {pid}

import mrworkserver

def handler(signum, frame):
  print("SIGHUP")
  
async def callback(ws, msgs):
  for m in msgs:
    print(m)

ws = mrworkserver.WorkServer(callback=callback)
ws.sighup = handler

ws.run(host="127.0.0.1",port=7100)

