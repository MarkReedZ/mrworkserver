
# pip install asyncmrq mrjson

import asyncio
from mrq.client import Client
import mrjson, mrpacker
import mrq

print (mrq.client.__version__)
print(mrq.__file__)

async def run(loop):
  c = Client()
  await c.connect(io_loop=loop,servers=[("127.0.0.1",7100)])

  # Test fetch
  if 1:
    b = mrjson.dumpb([15,1,0,30])
    ret = await c.get( b )
    print(mrpacker.unpack(ret))

  # Push some work
  if 0:
    msg = mrjson.dumpb([1,2,3,4,5,6,7,8,9,10])
    while 1:
      await c.push( 0, 0, msg, len(msg) )
      await asyncio.sleep(0.2)
    await c.flushcmd( 0, 0 )
    await asyncio.sleep(2)

  await c.close()

if __name__ == '__main__':
  loop = asyncio.get_event_loop()
  loop.run_until_complete(run(loop))
  loop.close()

# openssl req -new -newkey rsa:2048 -days 365 -nodes -x509 -keyout server.key -out server.crt
  #sc = ssl.create_default_context(ssl.Purpose.SERVER_AUTH)
  #sc.load_verify_locations('server.crt')
  #await c.connect(io_loop=loop,servers=[("127.0.0.1",7100)],ssl=sc)

