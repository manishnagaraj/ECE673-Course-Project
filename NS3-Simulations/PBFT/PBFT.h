/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 
 * PBFT .h file
 *
 * Manish Nagaraj <mnagara@purdue.edu>
 * ECE, Purdue University
 * 2020
 * 
 */

#include "ns3/core-module.h"
#include "ns3/application.h"
#include "ns3/internet-module.h"
#include "ns3/socket.h"

#include <openssl/evp.h>
#include <unordered_set>
#include <vector>

#include "link.h"

using namespace ns3;

class PBFT : public Application {
public:
	PBFT(); // Constructor
	static TypeId GetTypeId (void);
        virtual ~PBFT ();
	std :: list < Link* > outLinks;
	friend std::ostream& operator<<(std::ostream&, const PBFT&);
	void ConnectionSucceeded ( Ptr<Socket> socket );
	void ConnectionFailed ( Ptr<Socket> socket );
	void Send ( Link* link, Ptr < Packet > p );
	void SendEvent ( Ptr<Packet> packet );

protected:
	virtual void StartApplication (void);
	virtual void StopApplication (void);
        virtual void DoDispose (void);

private:
	void Setup(void);
	void HandleAccept (Ptr<Socket> s, const Address& from);
	void HandleRead ( Ptr<Socket> socket );
        void Propose ( Ptr<Packet> packet );
        void Precommit ( Ptr<Packet> packet );
	void Commit ( Ptr<Packet> packet );
        void SendAll ( Ptr<Packet> packet );
	bool Find ( Ptr<Packet> packet );
	Ptr<Socket> m_socket;
        uint16_t m_port;
	std :: unordered_set <std::string> recvd_msgs;
	std :: unordered_set <std::string> :: iterator it;
	std :: list<Ptr<Socket> > m_socketList; //!< the accepted sockets
};
