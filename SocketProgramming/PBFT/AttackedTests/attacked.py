import math
import time
import socket
import sys
import select
import pickle
import datetime
import pdb
from Crypto.Cipher import AES
from Crypto import Random
import os

print(os.getpid())
num_messages = dict()

def roundup(x):
    return int(math.ceil(x / 10.0)) * 10

N = 4

IV = b'\xa2\xae\x8b\xbd\xa5GJF\x13\xd9!\xd6\xad\x13\xe8\xa5'

cid = sys.argv[1]
SERVER = sys.argv[2]
PORT = sys.argv[3]
BYZANTINE = sys.argv[4]

keys = {
			1: {2: b'Fixteen syte key', 3: b'Bixteen yyte key', 'server': b'Sixteen byte key'}, 
			2: {1: b'Fixteen syte key', 3: b'Bilteen byte key', 'server': b'Sixteen syte key'},
			3: {1: b'Bixteen yyte key', 2: b'Bilteen byte key', 'server': b'Bixteen byte key'}
		}

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

intro = "REGISTER_REQUEST "+str(cid)
intro = intro.encode()

client.sendto(intro, (SERVER, int(PORT)))

response, address = client.recvfrom(1024)
neighbors = pickle.loads(response)
print(neighbors)

port_mapper = dict()
for i in neighbors:
	port_mapper[i[2]] = i[0]

port_mapper[int(PORT)] = 'server'

STAGE = 'PRE' 
match_message = ""
messages = []
NEW_MESSAGE = False
THRESHOLD = 5

while 1:
		inputready, outputready, exceptrdy = select.select([0, client], [],[], 0.5)

		for i in inputready:
			if NEW_MESSAGE:
				NEW_MESSAGE = False
				messages = []
				for i in num_messages:
					num_messages[i] = 0

			data, address = client.recvfrom(1024)
			port = address[1]
			try:
				num_messages[(address[0],address[1])] += 1
			except:
				num_messages[(address[0],address[1])] = 1

			#print((address[0], address[1]), num_messages[(address[0], address[1])])
			if num_messages[(address[0], address[1])] <= THRESHOLD:
				cipher = AES.new(keys[int(cid)][port_mapper[port]], AES.MODE_EAX, IV)
				plaintext = cipher.decrypt(data)
				data = plaintext[:-plaintext[-1]]
				messages.append(data.decode())
				print(messages)

		if not  (inputready or outputready or exceptrdy):
			if STAGE == 'PRE':
				match_message = "PREP10".encode()
				if BYZANTINE == 'n':
					length = 16 - (len(match_message) % 16)
					match_message += bytes([length])*length
					for i in neighbors:
						cipher = AES.new(keys[int(cid)][i[0]], AES.MODE_EAX, IV)
						ciphertext = cipher.encrypt(match_message)
						client.sendto(ciphertext, (SERVER, i[2]))

					cipher = AES.new(keys[int(cid)]['server'], AES.MODE_EAX, IV)
					ciphertext = cipher.encrypt(match_message)
					client.sendto(ciphertext, (SERVER, int(PORT)))

				if BYZANTINE == 'y':
					match = "PREP11".encode()
					length = 16 - (len(match) % 16)
					match += bytes([length])*length
					for i in neighbors:
						cipher = AES.new(keys[int(cid)][i[0]], AES.MODE_EAX, IV)
						ciphertext = cipher.encrypt(match)
						client.sendto(ciphertext, (SERVER, i[2]))

					cipher = AES.new(keys[int(cid)]['server'], AES.MODE_EAX, IV)
					ciphertext = cipher.encrypt(match)
					client.sendto(ciphertext, (SERVER, int(PORT)))
				
				STAGE = 'PREP'
				print("Sent prepare")
						

			elif STAGE == 'PREP':
				match_message = "PREP10"
				if len([match for match in messages if match == "PREP11"]) >= 100 or len([match for match in messages if match == "PREP10"]) >= 1:
					match_message = "COMMIT10".encode()
					if BYZANTINE == 'n':
						length = 16 - (len(match_message) % 16)
						match_message += bytes([length])*length
						for i in neighbors:
							cipher = AES.new(keys[int(cid)][i[0]], AES.MODE_EAX, IV)
							ciphertext = cipher.encrypt(match_message)
							client.sendto(ciphertext, (SERVER, i[2]))

						cipher = AES.new(keys[int(cid)]['server'], AES.MODE_EAX, IV)
						ciphertext = cipher.encrypt(match_message)
						client.sendto(ciphertext, (SERVER, int(PORT)))

					if BYZANTINE == 'y':
						match = "COMMIT11".encode()
						length = 16 - (len(match) % 16)
						match += bytes([length])*length
						for i in neighbors:
							cipher = AES.new(keys[int(cid)][i[0]], AES.MODE_EAX, IV)
							ciphertext = cipher.encrypt(match)
							client.sendto(ciphertext, (SERVER, i[2]))

						cipher = AES.new(keys[int(cid)]['server'], AES.MODE_EAX, IV)
						ciphertext = cipher.encrypt(match)
						client.sendto(ciphertext, (SERVER, int(PORT)))

					print("Sent commit")
					STAGE = 'COMMIT'


			elif STAGE == 'COMMIT':
				if BYZANTINE == 'n':
					match_message = "COMMIT10"
					if len([match for match in messages if match == "PREP11"]) >= 100 or len([match for match in messages if match == "COMMIT10"]) >= 2:
						NEW_MESSAGE = True
						print("committed ", match_message)
						STAGE = 'PRE'
						print("DONE")

				if BYZANTINE == 'y':
					NEW_MESSAGE = True
					print("committed ", match_message)
					STAGE = 'PRE'
					print("DONE")
				
				t = datetime.datetime.utcnow()
				now = t.second + t.microsecond/1000000.0
				future = roundup(now)
				sleeptime =  future - now
				time.sleep(sleeptime)