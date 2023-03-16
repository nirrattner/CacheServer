#!/usr/bin/env python3

import c2py
import os
import re
import socket
import struct
import time

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
INPUT_HEADER_FILE = CURRENT_DIRECTORY + '/../src/cache_communication_protocol.h'

START_PATTERN = re.compile('(typedef enum|typedef struct) {')
END_PATTERN = re.compile('} (.*)_t;')

class CacheProtocol:
  def __init__(self):
    self.protocol = {}
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
        lines.append(line)

        result = END_PATTERN.search(line)
        if result:
          name = result.group(1)
          text = '\n'.join(lines)
          if kind == 'struct':
            size = c2py.structSize(text, '>')
            vlist, pformat = c2py.structInfo(text, '>')
          elif kind == 'enum':
            size = 0
            lines = lines[1:-1]
            lines = [re.sub('[ ,\n]', '', value) for value in lines]
            lines = [value.split('=')[0] for value in lines]
            vlist = lines
            pformat = {value: index for index, value in enumerate(lines)}

          self.protocol[name] = dict(
              format=pformat,
              vlist=vlist,
              size=size,
              text=text,
              kind=kind)

  def define(self, type):
    buffer = b'\00' * self.protocol[type]['size']
    value = c2py.depack_bytearray_to_dict(buffer, self.protocol[type]['text'], '>')
    value['__TYPE'] = type
    return value

  def enum(self, type, value):
    return self.protocol[type.lower()]['format'][type.upper() + '__' + value.upper()]

  def pack_dict_to_bytearray(self, struct_instance, struct_definition, alignment="="):
    variable_list, pack_format = c2py.structInfo(struct_definition, alignment)
    variables = []

    for variable_name, array_length in variable_list:
      if array_length > 1:
        variables.append(struct_instance[variable_name])
        if array_length > len(struct_instance[variable_name]):
          variables += '\0' * (array_length - len(struct_instance[variable_name]))
      else:
        variables.append(struct_instance[variable_name])

    for index in range(len(variables)):
      if pack_format[index + 1] == 'c' and isinstance(variables[index], int):
        variables[index] = ('%c' % variables[index]).encode()
      elif isinstance(variables[index], str):
        variables[index] = variables[index].encode()
    buffer = struct.pack(pack_format, *variables)

    return buffer

  def pack(self, struct_instance):
    type = struct_instance['__TYPE']
    struct_instance_copy = dict(struct_instance)
    del struct_instance_copy['__TYPE']
    struct = self.pack_dict_to_bytearray(struct_instance_copy, self.protocol[type]['text'], '>')
    return struct

  def unpack(self, struct_instance, input_data):
    type = struct_instance['__TYPE']
    struct = c2py.depack_bytearray_to_dict(input_data, self.protocol[type]['text'], '>')
    struct['__TYPE'] = type
    return struct

protocol = CacheProtocol()

request_header = protocol.define('request_header')
request_header['version'] = 0xff 
request_header['flags'] = 123
request_header['type'] = protocol.enum('REQUEST_TYPE','PUT')

packed_header = protocol.pack(request_header)
print(f'packed_header: {packed_header}')


new_request_header = protocol.define('request_header')
new_request_header

new_request_header = protocol.unpack(new_request_header, packed_header)
print(f'new_request_header["version"] {new_request_header["version"]}')
print(f'new_request_header["flags"] {new_request_header["flags"]}')
print(f'new_request_header["type"] {new_request_header["type"]}')

