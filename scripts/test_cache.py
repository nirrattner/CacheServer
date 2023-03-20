import cache_client
import random
import string
import sys

from concurrent import futures

KEY_SIZE = 32
VALUE_SIZE = 256

ITERATION = 100
THREADS = 20

def random_characters(size):
  return bytes(''.join(random.choice(string.ascii_letters) for i in range(size)), 'utf-8')

def test_iteration(client):
  key = random_characters(KEY_SIZE)
  value = random_characters(VALUE_SIZE)

  client.ping()
  client.put(key, value)
  actual_value = client.get(key)
  if actual_value != value:
    raise Exception(f'expected {value} but got {actual_value}')

  client.delete(key)
  actual_value = client.get(key)
  if actual_value != None:
    raise Exception(f'expected {value} but got {actual_value}')

def test_thread_task(port):
  print(f'Starting...')
  client = cache_client.CacheClient(port=port)
  client.open()
  for iteration in range(ITERATION):
    test_iteration(client)
  client.close()
  print(f'Done')

if __name__ == '__main__':
  port = 8888
  if len(sys.argv) > 1:
    port = int(sys.argv[1])

  with futures.ThreadPoolExecutor(max_workers=THREADS) as executor:
    futures = [executor.submit(test_thread_task, port) for thread in range(THREADS)]
    for future in futures:
      future.result()

