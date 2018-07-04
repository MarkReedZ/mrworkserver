
import asyncio
from mrq.client import Client

async def run(loop):
  c = Client()
  await c.connect(io_loop=loop,servers=[("127.0.0.1",7100)],)

  bstr = "[1,2,3,4,5,6,7,8,9,10]".encode("utf-8")
  l = len(bstr)

  for x in range(10):
    await c.push( 0, 0, bstr, l )

  await asyncio.sleep(1)
  await c.close()

if __name__ == '__main__':
  loop = asyncio.get_event_loop()
  loop.run_until_complete(run(loop))
  loop.close()
