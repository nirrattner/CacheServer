import cache_protocol
import socket

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 8888
PROTOCOL = cache_protocol.CacheProtocol()

# TODO: Reconnect on socket failure?

class CacheClient:

  def __init__(self, host=DEFAULT_HOST, port=DEFAULT_PORT):
    self.host = host
    self.port = port

  def open(self):
    self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.socket.connect((self.host, self.port))

  def close(self):
    self.terminate()
    self.socket.close()

  def ping(self):
    request_header = PROTOCOL.allocate('request_header')
    request_header.fields['version'] = PROTOCOL.version
    request_header.fields['flags'] = PROTOCOL.enum('request_flag', 'keep_alive')
    request_header.fields['type'] = PROTOCOL.enum('request_type', 'ping')

    self.socket.send(request_header.pack())

    response_header = self.socket.recv(1)
    if response_header[0] != PROTOCOL.enum('response_type', 'pong'):
      raise Exception('ping failed ({response_header.hex()})')

  def put(self, key, value):
    request_header = PROTOCOL.allocate('request_header')
    request_header.fields['version'] = PROTOCOL.version
    request_header.fields['flags'] = PROTOCOL.enum('request_flag', 'keep_alive')
    request_header.fields['type'] = PROTOCOL.enum('request_type', 'put')

    key_value_arguments = PROTOCOL.allocate('key_value_arguments')
    key_value_arguments.fields['key_size'] = len(key)
    key_value_arguments.fields['value_size'] = len(value)

    self.socket.send(request_header.pack()\
        + key_value_arguments.pack()\
        + key\
        + value)

    response_header = self.socket.recv(1)
    if response_header[0] != PROTOCOL.enum('response_type', 'ok'):
      raise Exception('put failed ({response_header.hex()})')

  def get(self, key):
    request_header = PROTOCOL.allocate('request_header')
    request_header.fields['version'] = PROTOCOL.version
    request_header.fields['flags'] = PROTOCOL.enum('request_flag', 'keep_alive')
    request_header.fields['type'] = PROTOCOL.enum('request_type', 'get')

    key_arguments = PROTOCOL.allocate('key_arguments')
    key_arguments.fields['key_size'] = len(key)

    self.socket.send(request_header.pack()\
        + key_arguments.pack()\
        + key)

    response_header = self.socket.recv(1)
    if response_header[0] == PROTOCOL.enum('response_type', 'not_found'):
      return None

    if response_header[0] != PROTOCOL.enum('response_type', 'value'):
      raise Exception('get failed ({response_header.hex()})')

    value_arguments = PROTOCOL.allocate('value_arguments')
    value_arguments.unpack(self.socket.recv(value_arguments.size))
    return self.socket.recv(value_arguments.fields['value_size'])

  def delete(self, key):
    request_header = PROTOCOL.allocate('request_header')
    request_header.fields['version'] = PROTOCOL.version
    request_header.fields['flags'] = PROTOCOL.enum('request_flag', 'keep_alive')
    request_header.fields['type'] = PROTOCOL.enum('request_type', 'delete')

    key_arguments = PROTOCOL.allocate('key_arguments')
    key_arguments.fields['key_size'] = len(key)

    value_arguments = PROTOCOL.allocate('value_arguments')

    self.socket.send(request_header.pack()\
        + key_arguments.pack()\
        + key)

    response_header = self.socket.recv(1)
    if response_header[0] != PROTOCOL.enum('response_type', 'ok'):
      raise Exception('get failed ({response_header.hex()})')

  # TODO: Update to signal close?
  def terminate(self):
    request_header = PROTOCOL.allocate('request_header')
    request_header.fields['version'] = PROTOCOL.version
    request_header.fields['flags'] =  0
    request_header.fields['type'] = PROTOCOL.enum('request_type', 'ping')

    self.socket.send(request_header.pack())

    response_header = self.socket.recv(1)

