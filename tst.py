import difflib
import asyncio, ssl, sys, random
import mrworkserver, mrpacker

import time


num = 0
async def on_start(ws):
  print("on_start")
async def on_stop(ws):
  print("on_stop, num =",num)

async def reply_cb(ws, msgs): #conn, reply_id):
  for m in msgs:
    conn.reply( m[0], m[1], "ok" )

#l = []
def bb(m):
  global num
  num += 1
  #l.append(m)
  
async def callback(ws, msgs):
  start = time.time()
  global num
  l = []
  for m in msgs:
    #num += 1
    bb(m)
    #l.append(m)
    #print(m)
  print(num)
  print ("Processing",len(msgs),"took: ",(time.time() - start))

users = {}
def setcb(ws, k, v):
  global users
  users[k] = mrpacker.pack(v)

def fetchcb(ws, o):
  if o in users:
    return users[o]
  else:
    return b''
  #return mrpacker.pack( {"name":"mark"} )

collect_stats = False
ws = mrworkserver.WorkServer(seconds_to_gather=1,callback=callback,collect_stats=collect_stats,fetch_callback=fetchcb)
ws.setcb = setcb
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

