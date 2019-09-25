
# pip install asyncmrws mrjson

import asyncio
import mrpacker
import asyncmrws



async def run(loop):
  c = asyncmrws.Client()
  await c.connect(io_loop=loop,servers=[("127.0.0.1",7100)])

  # Test fetch
  if 1:
    print( mrpacker.unpack(await c.get( 0, mrpacker.pack([1,2,3]))) )

  # Push some work
  if 0:
    msg = mrpacker.pack([1,2,3,4,5,6,7,8,9,10])
    for x in range(10):
      print(x)
      await c.push( 0, 0, msg, len(msg) )
      await asyncio.sleep(1)

  await c.close()

if __name__ == '__main__':
  loop = asyncio.get_event_loop()
  loop.run_until_complete(run(loop))
  loop.close()

# openssl req -new -newkey rsa:2048 -days 365 -nodes -x509 -keyout server.key -out server.crt
  #sc = ssl.create_default_context(ssl.Purpose.SERVER_AUTH)
  #sc.load_verify_locations('server.crt')
  #await c.connect(io_loop=loop,servers=[("127.0.0.1",7100)],ssl=sc)

