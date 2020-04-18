import time
import socket
import sys
import select
import pickle
import pdb
from crypto.Cipher import AES
from crypto import Random

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


while 1:
		inputready, outputready, exceptrdy = select.select([0, client], [],[], 0.6)
		for i in inputready:
			if NEW_MESSAGE:
				NEW_MESSAGE = False
				messages = []

			data, address = client.recvfrom(1024)
			port = address[1]
			cipher = AES.new(keys[int(cid)][port_mapper[port]], AES.MODE_EAX, IV)
			plaintext = cipher.decrypt(data)
			data = plaintext[:-plaintext[-1]]
			print(messages)
			messages.append(data.decode())

		if not  (inputready or outputready or exceptrdy):
			if STAGE == 'PRE':
				match = "PREP11".encode()
				length = 16 - (len(match) % 16)
				match += bytes([length])*length
				i = neighbors[1]
				cipher = AES.new(keys[int(cid)][i[0]], AES.MODE_EAX, IV)
				ciphertext = cipher.encrypt(match)
				for x in range(100):
					client.sendto(ciphertext, (i[3], i[2]))
				match_message = "COMMIT10"
				if len([match for match in messages if match == match_message]):
					STAGE = 'COMMIT'
					print("Sent commit")

			if STAGE == 'COMMIT':			
				NEW_MESSAGE = True
				print("committed ", match_message)
				STAGE = 'PRE'
				print("DONE")
				time.sleep(10)