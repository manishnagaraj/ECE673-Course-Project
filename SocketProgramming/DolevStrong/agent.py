import socket, select, pickle
import base64
import hashlib
from crypto import Random
from crypto.Cipher import AES
import pickle
from crypto.Hash import SHA256
from crypto.Signature import PKCS1_v1_5
from crypto.PublicKey import RSA

def get_key(val): 
    for key, value in ports.items(): 
         if val == value: 
             return key

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


key = False
with open ("keys.pem", "r") as myfile:
    key = RSA.importKey(myfile.read())

signer = PKCS1_v1_5.new(key)

flag = True

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
								digest = SHA256.new()
								digest.update(match_message)
								sig = signer.sign(digest)
								data_string = pickle.dumps((match_message, sig))
								server.sendto(data_string, (addresses[client_ids[ind]], ports[client_ids[ind]]))

						if BYZANTINE == 'y':
							send_val = 10
							for ind, i in enumerate(addresses):
								match_message = str(send_val).encode()
								send_val+=1
								digest = SHA256.new()
								digest.update(match_message)
								sig = signer.sign(digest)
								data_string = pickle.dumps((match_message, sig))
								server.sendto(data_string, (addresses[client_ids[ind]], ports[client_ids[ind]]))


				# 		STAGE = 'PREP'
				# 		match_message = "PREP10"
				# 		print("Sent pre-prepare")
				# else:
				# 	port = address[1]
				# 	cipher = AES.new(keys[get_key(port)])
				# 	plaintext = cipher.decrypt(data)
				# 	data = plaintext[:-plaintext[-1]]
					
				# 	print(data.decode())
				# 	messages.append(data.decode())
					
				# 	if STAGE == 'PREP':
				# 		if len([match for match in messages if match == match_message]) >= 2:
				# 			STAGE = 'COMMIT'
				# 			if BYZANTINE == 'n':
				# 				for ind, i in enumerate(addresses):
				# 					match_message = "COMMIT10".encode()
				# 					length = 16 - (len(match_message) % 16)
				# 					match_message += bytes([length])*length
				# 					cipher = AES.new(keys[client_ids[ind]])
				# 					ciphertext = cipher.encrypt(match_message)
				# 					server.sendto(ciphertext, (addresses[client_ids[ind]], ports[client_ids[ind]]))
						
				# 			if BYZANTINE == 'y':
				# 				for ind, i in enumerate(addresses):
				# 					server.sendto("COMMIT11".encode(), (addresses[client_ids[ind]], ports[client_ids[ind]]))
							
				# 			print("Sent commit")

				# 	if STAGE == 'COMMIT':
				# 		match_message = "COMMIT10"
				# 		if len([match for match in messages if match == match_message]) >= 2:
				# 			print("committed", match_message)
				# 			STAGE = 'DONE'