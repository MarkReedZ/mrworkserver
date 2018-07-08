
import asyncio, ssl
import mrworkserver

async def callback(msgs):
  print("Callback:")
  for m in msgs:
    print(m)

ws = mrworkserver.WorkServer()
ws.cb = callback

#sc = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
#sc.load_cert_chain(certfile='cert/server.crt', keyfile='cert/server.key')

ws.run() #ssl=sc)

