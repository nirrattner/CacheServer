import socket

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 8888  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as connection_socket:
  connection_socket.connect((HOST, PORT))
  connection_socket.send(b'TEST')
  print('SENT')
  connection_socket.recv(4)

print(f"Received")
