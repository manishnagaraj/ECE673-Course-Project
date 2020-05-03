import socket
import os
import sys
import select
import pickle
import pdb
from Crypto.Cipher import AES
from Crypto import Random
from Crypto.Hash import SHA256
from Crypto.Signature import PKCS1_v1_5
from Crypto.PublicKey import RSA
import pickle

cid = sys.argv[1]
SERVER = sys.argv[2]
PORT = sys.argv[3]
BYZANTINE = sys.argv[4]


key = False
os.system("openssl genrsa -out keys"+cid+".pem 1024")

with open ("keys"+cid+".pem", "r") as myfile:
	my_key = RSA.importKey(myfile.read())

signer = PKCS1_v1_5.new(my_key)


client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

intro = "REGISTER_REQUEST "+str(cid)
intro = intro.encode()

client.sendto(intro, (SERVER, int(PORT)))

response, address = client.recvfrom(1024)
neighbors = pickle.loads(response)
print(neighbors)

for i in range(len(neighbors)):
	neighbor_id = neighbors[i][0]
	with open ("keys"+str(neighbor_id)+".pem", "r") as myfile:
		public_key = RSA.importKey(myfile.read())
	neighbors[i].append(public_key.publickey())
	neighbors[i].append(PKCS1_v1_5.new(public_key))

neighbor_dict = {}
for i in range(len(neighbors)):
	neighbor_dict[neighbors[i][0]] = neighbors[i][5]

neighbor_dict[int(cid)] = PKCS1_v1_5.new(my_key.publickey())
#[neighbor_id, 1, port, address, public key, verifier]


messages = []
stage = []

SENT = False

STAGE = 'PRE'
#verified = verifier.verify(digest, sig)

while 1:
		inputready, outputready, exceptrdy = select.select([0, client], [],[], 0.5)
		for i in inputready:
			data, address = client.recvfrom(1024)
			port = address[1]
			unpickled = pickle.loads(data)
			stage.append(unpickled[0])
			messages.append(unpickled[1])

		if not (inputready or outputready or exceptrdy):
			if BYZANTINE == 'n':
				if STAGE == 'PRE':
					if 'PRE' in stage:
						STAGE = 'COMMIT'
						index = stage.index("PRE")
						match_message = messages[index]
						digest = SHA256.new()
						digest.update(match_message)
						sig = signer.sign(digest)
						send_message = pickle.dumps((cid, match_message, sig))
						client.sendto(send_message, (SERVER, int(PORT)))

				elif STAGE == 'COMMIT':
					if 'COM' in stage:
						index = stage.index("COM")
						certificate = messages[index]

						senders = [i[0] for i in certificate]
						commit_messages = [i[1] for i in certificate]
						signatures = [i[2] for i in certificate]
						if len(set(commit_messages)) == 1:
							print("committed: ", commit_messages[0].decode())
						else:
							matches = []
							for i in range(len(signatures)):
								digest = SHA256.new()
								digest.update(commit_messages[i])
								matches.append(neighbor_dict[int(senders[i])].verify(digest, signatures[i]))
							if all(flag == True for flag in matches) == True:
								if len([i for i in commit_messages if i == match_message]) >= 2:
									print("committed: ", match_message.decode())
								else:
									print("no commit")
						STAGE = 'DONE'

			elif BYZANTINE == 'y':
				if STAGE == 'PRE':
					if 'PRE' in stage:
						STAGE = 'COMMIT'
						index = stage.index("PRE")
						match_message = "11".encode()
						digest = SHA256.new()
						digest.update(match_message)
						sig = signer.sign(digest)
						send_message = pickle.dumps((cid, match_message, sig))
						client.sendto(send_message, (SERVER, int(PORT)))

				elif STAGE == 'COMMIT':
					if 'COM' in stage:
						index = stage.index("COM")
						certificate = messages[index]

						senders = [i[0] for i in certificate]
						commit_messages = [i[1] for i in certificate]
						signatures = [i[2] for i in certificate]
						if len(set(commit_messages)) == 1:
							print("committed: ", commit_messages[0].decode())
						else:
							matches = []
							for i in range(len(signatures)):
								digest = SHA256.new()
								digest.update(commit_messages[i])
								matches.append(neighbor_dict[int(senders[i])].verify(digest, signatures[i]))
							if all(flag == True for flag in matches) == True:
								if len([i for i in commit_messages if i.decode() == match_message]) >= 2:
									print("committed: ", match_message)
								else:
									print("no commit")
						STAGE = 'DONE'