import socket, select, pickle
import base64
import hashlib
from crypto import Random
from crypto.Cipher import AES
import time

def get_key(val): 
    for key, value in ports.items(): 
         if val == value: 
             return key


N = 4

host = ''
port = 8081

IV = b'\xa2\xae\x8b\xbd\xa5GJF\x13\xd9!\xd6\xad\x13\xe8\xa5'

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

STAGE = 'PRE' 
match_message = ""
messages = []

running = 1


keys = {1: b'Sixteen byte key', 2: b'Sixteen syte key', 3: b'Bixteen byte key'}

flag = True
#conn.send(key.publickey().exportKey(format='PEM', passphrase=None, pkcs=1)) 
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
					
					if len(addresses) == N-1:
						for ind, i in enumerate(addresses):
							REGISTER_RESPONSE = pickle.dumps(clients[client_ids[ind]])
							server.sendto(REGISTER_RESPONSE, (addresses[client_ids[ind]], ports[client_ids[ind]]))
		

					if len(addresses) == N-1:
						for ind, i in enumerate(addresses):
							match_message = "PRE10".encode()
							length = 16 - (len(match_message) % 16)
							match_message += bytes([length])*length
							cipher = AES.new(keys[client_ids[ind]], AES.MODE_EAX, IV)
							ciphertext = cipher.encrypt(match_message)
							server.sendto(ciphertext, (addresses[client_ids[ind]], ports[client_ids[ind]]))
						STAGE = 'PREP'
						match_message = "PREP10"
						print("Sent pre-prepare")
				else:
					port = address[1]
					cipher = AES.new(keys[get_key(port)], AES.MODE_EAX, IV)
					plaintext = cipher.decrypt(data)
					data = plaintext[:-plaintext[-1]]
					
					print(data.decode())
					messages.append(data.decode())
					
					if STAGE == 'PREP':
						if len([match for match in messages if match == match_message]) >= N-2:
							STAGE = 'COMMIT'
							if BYZANTINE == 'n':
								for ind, i in enumerate(addresses):
									match_message = "COMMIT10".encode()
									length = 16 - (len(match_message) % 16)
									match_message += bytes([length])*length
									cipher = AES.new(keys[client_ids[ind]], AES.MODE_EAX, IV)
									ciphertext = cipher.encrypt(match_message)
									server.sendto(ciphertext, (addresses[client_ids[ind]], ports[client_ids[ind]]))
						
							if BYZANTINE == 'y':
								for ind, i in enumerate(addresses):
									server.sendto("COMMIT11".encode(), (addresses[client_ids[ind]], ports[client_ids[ind]]))
							
							print("Sent commit")

					if STAGE == 'COMMIT':
						match_message = "COMMIT10"
						if len([match for match in messages if match == match_message]) >= N-2:
							print("committed", match_message)
							time.sleep(10)
							del messages
							messages = []
							if len(addresses) == N-1:
								for ind, i in enumerate(addresses):
									match_message = "PRE10".encode()
									length = 16 - (len(match_message) % 16)
									match_message += bytes([length])*length
									cipher = AES.new(keys[client_ids[ind]], AES.MODE_EAX, IV)
									ciphertext = cipher.encrypt(match_message)
									server.sendto(ciphertext, (addresses[client_ids[ind]], ports[client_ids[ind]]))
								STAGE = 'PREP'
								match_message = "PREP10"
								print("Sent pre-prepare")