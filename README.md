# MrWorkServer
A simple clustered python 3.5+ asyncio based work server that uses the MrQ interface.

# Example

```python

import asyncio
import mrworkserver

async def callback(msgs):
  print("Callback:")
  for m in msgs:
    print(m)

ws = mrworkserver.WorkServer()
ws.cb = callback
ws.run()


```

# Example client

```python

# pip install asyncmrq mrjson

import asyncio
from mrq.client import Client
import mrjson

async def run(loop):
  c = Client()
  await c.connect(io_loop=loop,servers=[("127.0.0.1",7100)])

  msg = mrjson.dumpb([1,2,3,4,5,6,7,8,9,10])
  for x in range(10):
    await c.push( 0, 0, msg, len(msg) )

  await asyncio.sleep(1)
  await c.close()

if __name__ == '__main__':
  loop = asyncio.get_event_loop()
  loop.run_until_complete(run(loop))
  loop.close()

```


