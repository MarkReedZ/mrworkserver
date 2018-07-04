
import asyncio
import mrworkserver

def callback(msgs):
  print("Callback:")
  for m in msgs:
    print(m)

ws = mrworkserver.WorkServer()
ws.cb = callback
ws.run()

