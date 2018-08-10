
import asyncio, ssl
import mrworkserver

num = 0

async def callback(msgs):
  global num
  print("Callback:")
  for m in msgs:
    num += 1
    print(m)
  print( "num",num)

ws = mrworkserver.WorkServer(seconds_to_gather=5,callback=callback)

#sc = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
#sc.load_cert_chain(certfile='cert/server.crt', keyfile='cert/server.key')

ws.run() #ssl=sc)

