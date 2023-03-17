#!/usr/bin/env python3

import c2py
import os
import re
import socket
import struct
import time

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
INPUT_HEADER_FILE = CURRENT_DIRECTORY + '/../src/cache_protocol.h'

START_PATTERN = re.compile('(typedef enum|typedef struct) {')
END_PATTERN = re.compile('} (.*)_t;')

PACK_FORMAT = '<'

class CacheProtocolStructInstance:
  def __init__(self, definition):
    self.definition = definition
    self.size = definition['size']
    buffer = b'\00' * self.size
    self.fields = c2py.depack_bytearray_to_dict(buffer, self.definition['text'], PACK_FORMAT)

  def pack(self):
    variables = []
    pack_format = self.definition['pack_format']
    variable_list = self.definition['variable_list']

    for variable_name, array_length in variable_list:
      if array_length > 1:
        variables.append(self.fields[variable_name])
        if array_length > len(self.fields[variable_name]):
          variables += '\0' * (array_length - len(self.fields[variable_name]))
      else:
        variables.append(self.fields[variable_name])

    for index in range(len(variables)):
      if pack_format[index + 1] == 'c' and isinstance(variables[index], int):
        variables[index] = ('%c' % variables[index]).encode()
      elif isinstance(variables[index], str):
        variables[index] = variables[index].encode()
    buffer = struct.pack(pack_format, *variables)

    return buffer

  def unpack(self, input_data):
    self.fields = c2py.depack_bytearray_to_dict(input_data, self.definition['text'], PACK_FORMAT)

class CacheProtocol:
  def __init__(self):
    # TODO: Parse version from header
    self.version = 1
    self.definitions = {}
    with open(INPUT_HEADER_FILE, 'r') as input_file:
      lines = []
      for line in input_file.readlines():
        result = START_PATTERN.search(line)
        if result:
          if result.group(1) == 'typedef enum':
              kind = 'enum'
          else:
              kind = 'struct'
          lines = []
        line = line.split('//')[0].strip()
        if line:
          lines.append(line)

        result = END_PATTERN.search(line)
        if result:
          name = result.group(1)
          text = '\n'.join(lines)
          if kind == 'struct':
            size = c2py.structSize(text, PACK_FORMAT)
            variable_list, pack_format = c2py.structInfo(text, PACK_FORMAT)
          elif kind == 'enum':
            size = 0
            pack_format = None
            entries = [re.sub('[ ,\n]', '', value) for value in lines[1:-1]]

            next_value = 0
            variable_list = {}
            for entry in entries:
              entry_splits = entry.split('=')
              if len(entry_splits) > 1:
                next_value = eval(entry_splits[1])
              variable_list[entry_splits[0]] = next_value
              next_value += 1

          self.definitions[name] = dict(
              pack_format=pack_format,
              variable_list=variable_list,
              size=size,
              text=text)

  def allocate(self, type):
    return CacheProtocolStructInstance(self.definitions[type])

  def enum(self, type, value):
    return self.definitions[type.lower()]['variable_list'][type.upper() + '__' + value.upper()]

if __name__ == '__main__':
  protocol = CacheProtocol()

  request_header = protocol.allocate('request_header')
  request_header.fields['version'] = 0xff
  request_header.fields['flags'] = 123
  request_header.fields['type'] = protocol.enum('REQUEST_TYPE','PUT')

  packed_header_buffer = request_header.pack()
  print(f'packed_header_buffer: {packed_header_buffer}')

  new_request_header = protocol.allocate('request_header')

  new_request_header.unpack(packed_header_buffer)
  print(f'new_request_header.fields["version"] {new_request_header.fields["version"]}')
  print(f'new_request_header.fields["flags"] {new_request_header.fields["flags"]}')
  print(f'new_request_header.fields["type"] {new_request_header.fields["type"]}')

