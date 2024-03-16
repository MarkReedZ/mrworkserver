import mrworkserver

# In gather mode you must periodically call ws.process_messages to gather the collected messages
#   If set to False your callback will be called immediately as messages are received

num = 0

async def on_start(ws):
  print("Kicking off gather")
  ws.gather = asyncio.ensure_future( gather(ws) )

async def gather(ws):
  while True:
    await asyncio.sleep(5)
    ws.process_messages()

async def callback(ws, msgs):
  global num
  for m in msgs:
    num += 1
  print (f"Processed {num} messages")


ws = mrworkserver.WorkServer(gather=True,callback=callback,fetch_callback=fetchcb)
ws.on_start = on_start

ws.run(host="127.0.0.1",port=7100)

