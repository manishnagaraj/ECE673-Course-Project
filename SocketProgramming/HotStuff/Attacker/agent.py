import socket, select, pickle
import base64
import hashlib
from Crypto import Random
from Crypto.Cipher import AES
import pickle
from Crypto.Hash import SHA256
from Crypto.Signature import PKCS1_v1_5
from Crypto.PublicKey import RSA


host = ''
port = 8081

server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.bind((host, port))
clients = dict()
client_ids = []
inputs = [server]
addresses = dict()
ports = dict()
map_id = dict()
outputs =[]

BYZANTINE = 'n'

client_connections = []

match_message = ""
messages = []

running = 1

flag = True

message_recv = []

while running:
	inputready, outputready,exceptready = select.select(inputs, outputs,inputs, 5000)
	for s in inputready:
		if s == server:
			data, address = server.recvfrom(1024)
			if data:
				try:
					data.decode()
				except:
					flag = False

				if flag and data.decode()[:-2] == "REGISTER_REQUEST":
					id = int(data.decode()[-1])
					addresses[id] = address[0]
					ports[id] = address[1]
					
					for i in clients:
						clients[i].append([id, 1, ports[id], addresses[id]])
					clients[id] = []

					for i in clients:
						if i != id:
							clients[id].append([i, 1, ports[i], addresses[i]])
					
					client_ids.append(id)

					print(clients)
					print(client_ids)
					
					if len(addresses) == 3:
						for ind, i in enumerate(addresses):
							REGISTER_RESPONSE = pickle.dumps(clients[client_ids[ind]])
							server.sendto(REGISTER_RESPONSE, (addresses[client_ids[ind]], ports[client_ids[ind]]))

						if BYZANTINE == 'n':
							send_val = 10
							for ind, i in enumerate(addresses):
								match_message = str(send_val).encode()
								match_message = pickle.dumps(("PRE", match_message))
								server.sendto(match_message, (addresses[client_ids[ind]], ports[client_ids[ind]]))

						if BYZANTINE == 'y':
							send_val = 10
							for ind, i in enumerate(addresses):
								match_message = str(send_val).encode()
								send_val+=1
								match_message = pickle.dumps(("PRE", match_message))
								server.sendto(match_message, (addresses[client_ids[ind]], ports[client_ids[ind]]))
				else:
					unpickled = pickle.loads(data)
					message_recv.append(unpickled)
					if len(message_recv) == 3:
						for ind, i in enumerate(addresses):
								match_message = pickle.dumps(("COM", message_recv))
								server.sendto(match_message, (addresses[client_ids[ind]], ports[client_ids[ind]]))

