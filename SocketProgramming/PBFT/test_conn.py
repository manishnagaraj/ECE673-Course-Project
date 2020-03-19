import socket
import sys
import select
import pickle
import pdb

cid = sys.argv[1]
SERVER = sys.argv[2]
PORT = sys.argv[3]
BYZANTINE = sys.argv[4]

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

intro = "REGISTER_REQUEST "+str(cid)

client.sendto(intro.encode(), (SERVER, int(PORT)))
response, address = client.recvfrom(1024)
neighbors = pickle.loads(response)
print(neighbors)

STAGE = 'PRE' 
match_message = ""
messages = []

while 1:
		inputready, outputready, exceptrdy = select.select([0, client], [],[], 0.5)
		for i in inputready:
			data, address = client.recvfrom(1024)
			print(data.decode())
			messages.append(data)
		if not  (inputready or outputready or exceptrdy):
			if STAGE == 'PRE':
				match_message = "PREP10"
				#pdb.set_trace()
				if BYZANTINE == 'n':
					for i in neighbors:
						client.sendto(match_message.encode(), (i[3], i[2]))
					client.sendto(match_message.encode(), (SERVER, int(PORT)))

				if BYZANTINE == 'y':
					for i in neighbors:
						client.sendto("PREP11".encode(), (i[3], i[2]))
					client.sendto("PREP11".encode(), (SERVER, int(PORT)))
				
				STAGE = 'PREP'
				print("Sent prepare")
						

			elif STAGE == 'PREP':
				if len([match for match in messages if match == match_message]) == 1:
					STAGE = 'COMMIT'
					match_message = "COMMIT10"
					if BYZANTINE == 'n':
						for i in neighbors:
							client.sendto(match_message.encode(), (i[3], i[2]))
						client.sendto(match_message.encode(), (SERVER, int(PORT)))
				
					if BYZANTINE == 'y':
						for i in neighbors:
							client.sendto("COMMIT11".encode(), (i[3], i[2]))
						client.sendto("COMMIT11".encode(), (SERVER, int(PORT)))

					print("Sent commit")


			elif STAGE == 'COMMIT':
				if BYZANTINE == 'n':
					if len([match for match in messages if match == match_message]) == 2:
						print("committed ", match_message)
				STAGE = 'DONE'