import socket
import sys
import select
import pickle
import pdb
from crypto.Cipher import AES
from crypto import Random
from crypto.Hash import SHA256
from crypto.Signature import PKCS1_v1_5
from crypto.PublicKey import RSA
import datetime
import pickle
import math
import time

def roundup(x):
    return int(math.ceil(x / 10.0)) * 10

cid = sys.argv[1]
SERVER = sys.argv[2]
PORT = sys.argv[3]
BYZANTINE = sys.argv[4]


key = False
with open ("keys.pem", "r") as myfile:
    key = RSA.importKey(myfile.read())

public_key = key.publickey()
verifier = PKCS1_v1_5.new(public_key)

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

intro = "REGISTER_REQUEST "+str(cid)
intro = intro.encode()

client.sendto(intro, (SERVER, int(PORT)))

response, address = client.recvfrom(1024)
neighbors = pickle.loads(response)
print(neighbors)

messages = []
signs = []

RECV = False
SENT = False
leaderdata = None
STAGE = 'PRE'

NEW_MESSAGE = False
#verified = verifier.verify(digest, sig)

while 1:
		inputready, outputready, exceptrdy = select.select([0, client], [],[], 0.5)
		for i in inputready:
			data, address = client.recvfrom(1024)
			port = address[1]
			if str(port) == str(PORT):
				leaderdata = data
				RECV = True
			unpickled = pickle.loads(data)
			messages.append(unpickled[0].decode())
			signs.append(unpickled[1])
			print(messages)
			if len(messages) >= 3:
				if len(set(messages)) == 1:
					print("committed: ", messages[0])
				else:
					sender = []
					for i in range(len(signs)):
						digest = SHA256.new()
						digest.update(messages[i].encode())
						sender.append(verifier.verify(digest, signs[i]))
					if all(flag == True for flag in sender) == True:
						print("no commit")
					else: 
						signed_messages = [messages[m] for m in range(len(sender)) if sender[m] == True]
						if len(set(signed_messages)) == 1:
							print("committed: ", signed_messages[0])
						else:
							print("no commit")
				STAGE = 'DONE'

		if not  (inputready or outputready or exceptrdy):
			if NEW_MESSAGE:
				NEW_MESSAGE = False
				messages = []
				signs = []
				unpickled = pickle.loads(leaderdata)
				messages.append(unpickled[0].decode())
				signs.append(unpickled[1])

			if BYZANTINE == 'n' and not SENT and RECV:
				for i in neighbors:
					client.sendto(leaderdata, (i[3], i[2]))
				SENT = True

			if BYZANTINE == 'y' and not SENT:
				match_message = str("11").encode()
				digest = SHA256.new()
				digest.update(match_message)
				data_string = pickle.dumps((match_message, signs[0]))
				for i in neighbors:
					client.sendto(data_string, (i[3], i[2]))
				SENT = True

			if SENT and STAGE == 'DONE':
				t = datetime.datetime.utcnow()
				now = t.second + t.microsecond/1000000.0
				future = roundup(now)
				sleeptime =  future - now
				time.sleep(sleeptime)
				SENT = False
				NEW_MESSAGE = True
				STAGE = 'PRE'
					








			# 		m = 0
			# 		while m in range(len(messages)):
			# 			match_message = messages[m].encode()
			# 			length = 16 - (len(match_message) % 16)
			# 			match_message += bytes([length])*length
			# 			for i in neighbors:
			# 				cipher = AES.new(keys[int(cid)][i[0]])
			# 				ciphertext = cipher.encrypt(match_message)
			# 				client.sendto(ciphertext, (i[3], i[2]))
			# 			messages.pop(m)
			# 			m+=1
				
			# 	if BYZANTINE == 'y':
			# 		m = 0
			# 		while m in range(len(messages)):
			# 			match_message = "11".encode()
			# 			length = 16 - (len(match_message) % 16)
			# 			match_message += bytes([length])*length
			# 			for i in neighbors:
			# 				cipher = AES.new(keys[int(cid)][i[0]])
			# 				ciphertext = cipher.encrypt(match_message)
			# 				client.sendto(ciphertext, (i[3], i[2]))
			# 			messages.pop(m)
			# 			m+=1
			# 	STAGE += 1
			# print("DONE:", messages)




			# if STAGE == 'PRE':
			# 	match_message = "PREP10".encode()
			# 	if BYZANTINE == 'n':
			# 		length = 16 - (len(match_message) % 16)
			# 		match_message += bytes([length])*length
			# 		for i in neighbors:
			# 			cipher = AES.new(keys[int(cid)][i[0]])
			# 			ciphertext = cipher.encrypt(match_message)
			# 			client.sendto(ciphertext, (i[3], i[2]))

			# 		cipher = AES.new(keys[int(cid)]['server'])
			# 		ciphertext = cipher.encrypt(match_message)
			# 		client.sendto(ciphertext, (SERVER, int(PORT)))

			# 	if BYZANTINE == 'y':
			# 		match = "PREP11".encode()
			# 		length = 16 - (len(match) % 16)
			# 		match += bytes([length])*length
			# 		for i in neighbors:
			# 			cipher = AES.new(keys[int(cid)][i[0]])
			# 			ciphertext = cipher.encrypt(match)
			# 			client.sendto(ciphertext, (i[3], i[2]))

			# 		cipher = AES.new(keys[int(cid)]['server'])
			# 		ciphertext = cipher.encrypt(match)
			# 		client.sendto(ciphertext, (SERVER, int(PORT)))
				
			# 	STAGE = 'PREP'
			# 	print("Sent prepare")
						

			# elif STAGE == 'PREP':
			# 	match_message = "PREP10"
			# 	if len([match for match in messages if match == match_message]) >= 1:
			# 		match_message = "COMMIT10".encode()
			# 		if BYZANTINE == 'n':
			# 			length = 16 - (len(match_message) % 16)
			# 			match_message += bytes([length])*length
			# 			for i in neighbors:
			# 				cipher = AES.new(keys[int(cid)][i[0]])
			# 				ciphertext = cipher.encrypt(match_message)
			# 				client.sendto(ciphertext, (i[3], i[2]))

			# 			cipher = AES.new(keys[int(cid)]['server'])
			# 			ciphertext = cipher.encrypt(match_message)
			# 			client.sendto(ciphertext, (SERVER, int(PORT)))

			# 		if BYZANTINE == 'y':
			# 			match = "COMMIT11".encode()
			# 			length = 16 - (len(match) % 16)
			# 			match += bytes([length])*length
			# 			for i in neighbors:
			# 				cipher = AES.new(keys[int(cid)][i[0]])
			# 				ciphertext = cipher.encrypt(match)
			# 				client.sendto(ciphertext, (i[3], i[2]))

			# 			cipher = AES.new(keys[int(cid)]['server'])
			# 			ciphertext = cipher.encrypt(match)
			# 			client.sendto(ciphertext, (SERVER, int(PORT)))

			# 		print("Sent commit")
			# 		STAGE = 'COMMIT'


			# elif STAGE == 'COMMIT':
			# 	if BYZANTINE == 'n':
			# 		if len([match for match in messages if match == match_message]) >= 2:
			# 			print("committed ", match_message)
			# 	STAGE = 'DONE'
			# 	print(STAGE)