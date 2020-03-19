/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 
 * This is a first trial of simulations of SMR protocols 
 * using NS3
 *
 * PBFT Algorithm - Simulator Code
 * Needs to be linked with application
 * With the help of PBFT-helper and PBFT 
 *
 * Manish Nagaraj <mnagara@purdue.edu>
 * ECE, Purdue University
 * 2020
 * 
 */

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/application.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "PBFT.h"

using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED (PBFT);
NS_LOG_COMPONENT_DEFINE ("PBFT");

//Typer Id of the function
TypeId
PBFT :: GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PBFT")
    .SetParent<Application> ()
    .SetGroupName("SMR_Protocols")
    .AddConstructor<PBFT> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&PBFT::m_port),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

//Constructor
PBFT :: PBFT()
{
	NS_LOG_FUNCTION (this);
        
}

PBFT::~PBFT()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
}
//Dispose as in get rid of?
void
PBFT::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}



//To see if packet is found in buffer
PBFT :: Find (
	Ptr<Packet> packet
)
{
	it = recvd_msgs .find ( packet );
	return it != recvd_msgs .end();
}

//Adding packet to a set
void AddPacketToSet (
	std :: unordered_set <std::string>* packet_set,
	Ptr<Packet> packet
) 
{
	packet_set->insert( packet );
}

//To send packages
void
PBFT :: Send ( Link* link, Ptr < Packet > p )
{
	std :: list < Ipv4Address > :: iterator ait;
	for ( ait = link->sinks.begin() ; ait != link->sinks.end() ; ++ait ) {
		TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
		Ptr<Socket> temp_socket = Socket::CreateSocket ( GetNode() , tid );
		if (temp_socket->Bind () == -1) {
			std :: cout << "Failed to bind to the sending socket"
				<< std :: endl;
			return;
		}
		temp_socket->SetAllowBroadcast (true);
		temp_socket->ShutdownRecv ();
		temp_socket->SetConnectCallback (
				MakeCallback (&PBFT::ConnectionSucceeded, this),
				MakeCallback (&PBFT::ConnectionFailed, this));
		temp_socket->Connect (InetSocketAddress( *ait, 10));
		temp_socket->Send ( p );
                temp_socket->close();
	}
}

// Starting Application
void
PBFT :: StartApplication (void)
{
	NS_LOG_FUNCTION (this);
	//setup the model
	Setup();
	//Leader should initiate protocol
	if ( GetNode()->GetId() == 0 ) {
		std :: cout << "This node is the leader\n";
		//std :: string id_buffer = std::to_string( GetNode()->GetId() );
                std :: string id_buffer = "1,Hello";
		//Create packet
		Ptr<Packet> packet1 = Create<Packet> ((unsigned char*)id_buffer.c_str(),
				id_buffer.length() );
		//Send Packet
		Simulator :: Schedule ( 
			Seconds (2.0) , 
			& PBFT :: SendEvent , 
			this ,
			packet1
		);
	} else {
		std :: cout << "This node is the receiver\n";
	}
}

void
PBFT :: StopApplication (void)
{
	NS_LOG_FUNCTION (this);
        m_socket->Close();
}

std::ostream& operator<<(std::ostream& strm, const PBFT &client) 
{
    strm << "PBFT Client";
	return strm;
}

//Setup The model
void
PBFT :: Setup(void)
{
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	//Create socket to listen
     if (m_socket == 0)
	m_socket = Socket::CreateSocket (GetNode (), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
	if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }//if multicasting
    }//if m_socket==0

	m_socket->Listen();
	//Ensure it can not send
	m_socket->ShutdownSend ();
	//If you accept connection
	m_socket->SetAcceptCallback (
			MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
			MakeCallback (&PBFT :: HandleAccept, this));
	//If you receive message
	m_socket->SetRecvCallback (MakeCallback (&PBFT :: HandleRead, this));
	std :: cout << "Listening\n";
}

//If you receive connection
void
PBFT :: HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&PBFT :: HandleRead, this));
  m_socketList.push_back (s);
#ifdef DEBUG_MODE
  std :: cout << "Accepted a connection from " << from << "\n";
#endif
}

//If you receive message
void
PBFT :: HandleRead ( Ptr<Socket> socket )
{
	NS_LOG_FUNCTION(this << socket);
	Ptr<Packet> packet;
	Address from;
	while((packet = socket->RecvFrom(from))) {
		uint64_t total = packet->GetSize();
                if ( Find( packet) ) {
			std :: cout << "Packet in buffer. Not adding.\n";
		}//if package is already there
                else{
                        std :: cout << "Packet not in buffer. Adding..\n";
			AddPacketToSet ( &recvd_msgs , packet );
                        std :: string Value_recvd = packet->ToString();
                        if (Value_recvd(0)==1)
                                Propose(packet);
                        else if (Value_recvd(0)==2)
                                Precommit(packet);
                        else
                                Commit(packet);
                }//else
        }//while loop
}//Handle Read

void
PBFT :: Propose (Ptr<Packet> packet){
        std :: string Value_recvd = packet->ToString();
        Value_recvd(0)=2;
        SendAll( packet );
        
}

void
PBFT :: Precommit (Ptr<Packet> packet){
//       if(m_pre_count <2){
//                m_pre_count++;
//        }
//        else{
                std :: string Value_recvd = packet->ToString();
                Value_recvd(0)=3;
                SendAll( packet );
//             }        
}

void
PBFT :: Commit (Ptr<Packet> packet){
//       if(m_pre_count <2){
//                m_pre_count++;
//        }
//        else{
                SendAll( packet );
//             }        
}




		//std :: cout << "Received a packet at node " << GetNode()->GetId()+1 <<
		//	". Size: " << total << std :: endl;
/*		if ( Find( packet) ) {
			std :: cout << "Packet in buffer. Not adding.\n";
		} else {
			std :: cout << "Packet not in buffer. Adding..\n";
			AddPacketToSet ( &recvd_msgs , packet );
			Simulator :: Schedule ( 
				Seconds ( 1.0 ), 
				& PBFT :: SendEvent ,
				this ,
				packet
			);
		}
	}
}
*/


//Define Sending Event
void
PBFT :: SendEvent ( Ptr<Packet> packet ) {
	SendAll( packet );
}

//Connection Succeed
void
PBFT :: ConnectionSucceeded(Ptr<Socket> socket)
{
	NS_LOG_FUNCTION ( "Connection Succeeded\n" );
#ifdef DEBUG_MODE
	std :: cout << "Connected\n";
#endif 
}

//Connection Failure
void
PBFT :: ConnectionFailed(Ptr<Socket> socket)
{
	NS_LOG_FUNCTION ( "Connection Failed\n" );
#ifdef DEBUG_MODE
	std :: cout << "Connection Failed\n";
#endif 
}

//Send All
void
PBFT :: SendAll ( Ptr < Packet > packet )
{
	std :: list < Link* > :: iterator lit;
	int id = GetNode()->GetId();
	for ( lit = outLinks.begin() ; lit!= outLinks.end(); ++lit ) {
		printf ( " Sending packet from Node %d\n" , id );
		Send ( *lit, packet );
	}
}
