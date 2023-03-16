import cache_protocol
import socket

HOST = "127.0.0.1"
PORT = 8888
PROTOCOL = cache_protocol.CacheProtocol()

def ping(connection_socket, flags=0):
  request_header = PROTOCOL.allocate('request_header')
  request_header.fields['version'] = 1
  request_header.fields['flags'] = flags
  request_header.fields['type'] = PROTOCOL.enum('request_type', 'ping')

  print('ping...')
  connection_socket.send(request_header.pack())

  result = connection_socket.recv(1)
  if result[0] == PROTOCOL.enum('response_type', 'pong'):
    print('pong')
  else:
    print(f'ERROR: pong failed ({result.hex()})')

def put(connection_socket, key, value, flags=0):
  request_header = PROTOCOL.allocate('request_header')
  request_header.fields['version'] = 0
  request_header.fields['flags'] = flags
  request_header.fields['type'] = PROTOCOL.enum('request_type', 'put')

  key_value_arguments = PROTOCOL.allocate('key_value_arguments')
  key_value_arguments.fields['key_size'] = len(key)
  key_value_arguments.fields['value_size'] = len(value)

  print('put...')
  connection_socket.send(request_header.pack())
  connection_socket.send(key_value_arguments.pack())
  connection_socket.send(key)
  connection_socket.send(value)

  response_header = connection_socket.recv(1)
  if response_header[0] == PROTOCOL.enum('response_type', 'ok'):
    print('put successful')
  else:
    print(f'ERROR: put failed ({response_header.hex()})')

def get(connection_socket, key, flags=0):
  request_header = PROTOCOL.allocate('request_header')
  request_header.fields['version'] = 1
  request_header.fields['flags'] = flags
  request_header.fields['type'] = PROTOCOL.enum('request_type', 'get')

  key_arguments = PROTOCOL.allocate('key_arguments')
  key_arguments.fields['key_size'] = len(key)

  value_arguments = PROTOCOL.allocate('value_arguments')

  print('get...')
  connection_socket.send(request_header.pack())
  connection_socket.send(key_arguments.pack())
  connection_socket.send(key)

  response_header = connection_socket.recv(1)
  if response_header[0] == PROTOCOL.enum('response_type', 'value'):
    value_arguments.unpack(connection_socket.recv(value_arguments.size))
    return connection_socket.recv(value_arguments.fields['value_size'])
  else:
    print(f'ERROR: get failed ({response_header.hex()})')

def delete(connection_socket, key, flags=0):
  request_header = PROTOCOL.allocate('request_header')
  request_header.fields['version'] = 1
  request_header.fields['flags'] = flags
  request_header.fields['type'] = PROTOCOL.enum('request_type', 'delete')

  key_arguments = PROTOCOL.allocate('key_arguments')
  key_arguments.fields['key_size'] = len(key)

  value_arguments = PROTOCOL.allocate('value_arguments')

  print('delete...')
  connection_socket.send(request_header.pack())
  connection_socket.send(key_arguments.pack())
  connection_socket.send(key)

  response_header = connection_socket.recv(1)
  if response_header[0] == PROTOCOL.enum('response_type', 'ok'):
    print('delete successful')
  else:
    print(f'ERROR: delete failed ({response_header.hex()})')

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as connection_socket:
  connection_socket.connect((HOST, PORT))

  put(connection_socket, b'key-1', b'value-1', flags=PROTOCOL.enum('request_flag', 'keep_alive'))

  result = get(connection_socket, b'key-1', flags=PROTOCOL.enum('request_flag', 'keep_alive'))
  print(f'result {result}')

  delete(connection_socket, b'key-1', flags=PROTOCOL.enum('request_flag', 'keep_alive'))

  result = get(connection_socket, b'key-1', flags=0)
  print(f'result {result}')

