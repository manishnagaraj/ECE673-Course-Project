# import socket, select, pickle

# host = ''
# port = 8081

# server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# server.bind((host, port))
# server.listen(5)
# clients = []
# client_ids = set()
# inputs = [server]
# addresses = []
# ports = []
# map_id = dict()
# outputs =[]

# running = 1
# client_count = 0
# while running:
#     inputready, outputready,exceptready = select.select(inputs, outputs, inputs, 50000)
#     for s in inputready:
#         if s == server:
#             client, address = server.accept()
#             print('got connection %d from %s' % (client.fileno(), address))
#             inputs.append(client)
#             data = client.recv(1024)
#             if data:
#                 print(data.decode())
#                 print("sending REGISTER_RESPONSE")
#                 if address[0] not in addresses or address[1] not in ports:
#                     addresses.append(address[0])
#                     ports.append(address[1])
#                     clients.append(client)
#                     client_count += 1
#                 REGISTER_RESPONSE = pickle.dumps("Hello")
#                 print(client_count)
#                 client.send(REGISTER_RESPONSE)
#                 if client_count == 3:
#                     for client_send in clients:
#                         client_send.send("COMMIT10")
				
#                 # if data.decode()[:-2] == "REGISTER_REQUEST":
#                 #     print("sending REGISTER_RESPONSE")
#                 #     id = int(data.decode()[-1])
#                 #     map_id[client.fileno()] = id
#                 #     addresses[id] = address[0]
#                 #     ports[id] = address[1]

#                 #     for i in clients:
#                 #         clients[i].append([id, 1, ports[id], addresses[id]])

#                 #     clients[id] = []
					

#                 #     for i in clients:
#                 #         if i != id:
#                 #             clients[id].append([i, 1, ports[i], addresses[i]])
					
#                 #     REGISTER_RESPONSE = pickle.dumps(clients[id])
#                 #     client.send(REGISTER_RESPONSE)

#                 #     client_ids.add(id)

#                 #     print(clients)
#                 #     print(client_ids)
			
#                 #     outputs.append(client)
#         else:
#             data = s.recv(1024)
#             if data:
#                 outputs.append(s)
#                 print(data.decode())

#                 # if s in outputs:
#                 #     s.send(data)
#                 #     outputs.remove(s)



import socket, select, pickle

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

STAGE = 'PRE' 
match_message = ""
messages = []

running = 1

while running:
	inputready, outputready,exceptready = select.select(inputs, outputs,inputs, 5000)
	for s in inputready:
		if s == server:
			data, address = server.recvfrom(1024)
			if data:
				if data.decode()[:-2] == "REGISTER_REQUEST":
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
					
					if len(addresses) == 3:
						for ind, i in enumerate(addresses):
							server.sendto("PRE10".encode(), (addresses[client_ids[ind]], ports[client_ids[ind]]))
						STAGE = 'PREP'
						match_message = "PREP10"
						print("Sent pre-prepare")
				else:
					if STAGE == 'PREP':
						print(data)
						messages.append(data.decode())
						if len([match for match in messages if match == match_message]) == 2:
							STAGE = 'COMMIT'
							if BYZANTINE == 'n':
								for ind, i in enumerate(addresses):
									server.sendto("COMMIT10", (addresses[client_ids[ind]], ports[client_ids[ind]]))
						
							if BYZANTINE == 'y':
								for ind, i in enumerate(addresses):
									server.sendto("COMMIT11", (addresses[client_ids[ind]], ports[client_ids[ind]]))
							
							match_message = "COMMIT10"
							print("Sent commit")

					if STAGE == 'COMMIT':
						print(data)
						messages.append(data)
						if len([match for match in messages if match == match_message]) == 2:
							print("committed", match_message)
							STAGE = 'DONE'
		
		else:
			data = s.recv(1024)
			if data:
				outputs.append(s)
				#print(data.decode())

		#         if s in outputs:
		#                 s.send(data)
		#                 outputs.remove(s)
		#     else:                    
		#         print(map_id[s.fileno()])
		#         del clients[map_id[s.fileno()]]
		#         client_ids.remove(map_id[s.fileno()])
		#         for i in clients:
		#             for x in clients[i]:
		#                 if x[0] == map_id[s.fileno()]:
		#                     x[1] = 0
		#         s.close()
		#         inputs.remove(s)
		#         outputs.remove(s)
		#         msg = 'Left'                   
		#         for o in outputs:
		#             o.send(msg.encode())
		#         print(clients)
		#         print(client_ids)



		#             #send(o, msg)
		
		# """for s in outputready:
		#     s.send('gotcha'.encode())
		#     outputs.remove(s)
		#     inputs.append(s)"""


