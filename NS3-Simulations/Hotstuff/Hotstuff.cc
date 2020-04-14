/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Trying hotstuff
 *
 * Basing it on udpechoclient and Hotstuff i developed 
 *
 * Author: Manish Nagaraj (mnagara@purdue.edu)
 * 	   Purdue University
 */

#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"

#include "Hotstuff.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Hotstuff");

NS_OBJECT_ENSURE_REGISTERED(Hotstuff);

//Type ID definition
TypeId
Hotstuff::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::Hotstuff")
	    .SetParent<Application> ()
	    .SetGroupName("Applications")
            .AddConstructor<Hotstuff> ()
	    .AddAttribute("DataRate","The data rate in on state.",
			      DataRateValue (DataRate ("500kb/s")),
			      MakeDataRateAccessor (&Hotstuff::m_cbrRate),
			      MakeDataRateChecker ())
	    .AddAttribute ("PacketSize", "The size of packets sent in on state",
			   UintegerValue (512),
			   MakeUintegerAccessor (&Hotstuff::m_pktSize),
			   MakeUintegerChecker<uint32_t> (1))
	    .AddAttribute ("NumberFaults", "The number of faulty nodes in the system",
			   UintegerValue (1),
			   MakeUintegerAccessor (&Hotstuff::m_faults),
			   MakeUintegerChecker<uint64_t> (1))
	    .AddAttribute ("Local",
			   "The Address on which to Bind the rx socket.",
			   AddressValue (),
			   MakeAddressAccessor (&Hotstuff::m_local),
			   MakeAddressChecker ())
	    .AddAttribute ("Protocol",
			   "The type id of the protocol to use for the rx socket.",
			   TypeIdValue (UdpSocketFactory::GetTypeId ()),
			   MakeTypeIdAccessor (&Hotstuff::m_tid),
			   MakeTypeIdChecker ())
	    .AddTraceSource ("Rx",
			     "A packet has been received",
			     MakeTraceSourceAccessor (&Hotstuff::m_rxTrace),
			     "ns3::Packet::AddressTracedCallback")
	    .AddTraceSource ("RxWithAddresses", "A packet has been received",
			     MakeTraceSourceAccessor (&Hotstuff::m_rxTraceWithAddresses),
			     "ns3::Packet::TwoAddressTracedCallback")
	    .AddTraceSource ("Tx", "A new packet is created and is sent",
			     MakeTraceSourceAccessor (&Hotstuff::m_txTrace),
			     "ns3::Packet::TracedCallback")
	    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
			     MakeTraceSourceAccessor (&Hotstuff::m_txTraceWithAddresses),
			     "ns3::Packet::TwoAddressTracedCallback")
	  ;
	return tid;
}//tid

//Constructor
Hotstuff::Hotstuff ()
{
  NS_LOG_FUNCTION (this);
  m_socket_Rx = 0;
  m_socket_Tx = 0;
  m_quorum = 0;
}

//~constructor
Hotstuff::~Hotstuff()
{
  NS_LOG_FUNCTION (this);
}

//Get listening socket
Ptr<Socket>
Hotstuff::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket_Rx;
}

//Get transmit socket
Ptr<Socket>
Hotstuff::GetSendingSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket_Tx;
}

//Count of quorum
uint64_t Hotstuff::GetCnt ()
{
      NS_LOG_FUNCTION (this);
      return m_quorum;
}

//increment cnt
void Hotstuff::AddCnt (void)
{
      NS_LOG_FUNCTION (this);
      m_quorum++;
}

//Get list of accepted sockets
std::list<Ptr<Socket> >
Hotstuff::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

// Write function to check quorum
//ToDo

//DoDispose
void Hotstuff::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_socket_Rx = 0;
  m_socket_Tx = 0;
  m_socketList.clear ();
  m_connected = false;

  // chain up
  Application::DoDispose ();
}

//StartApplication
void Hotstuff::StartApplication()
{
  NS_LOG_FUNCTION(this);
  //CreateSocket
  if (!m_socket_Rx)
    {
      m_socket_Rx = Socket::CreateSocket (GetNode (), m_tid);
      if (m_socket_Rx->Bind (m_local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }//if not a success
      m_socket_Rx->Listen ();
      m_socket_Rx->ShutdownSend ();
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket_Rx);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }//if it is a udpsocket
          else
            {
              NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }//otherwise
        }//if we are using multicast addresses
    }//if packet has not been created

  //Is the node the initial leader? Then send initial package
  if(GetNode()->GetId()==0){
        //Ptr<Packet> packet1 = Create<Packet> (m_pktSize);
        std::ostringstream msg; 
        msg << "0,Hello!" << '\0';
        Ptr<Packet> packet1 = Create<Packet> ((uint8_t*) msg.str().c_str(), msg.str().length());
        SendEvent(packet1);  
   }

  //Listen
  m_socket_Rx->SetRecvCallback (MakeCallback (&Hotstuff::HandleRead, this));
  m_socket_Rx->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&Hotstuff::HandleAccept, this));
  m_socket_Rx->SetCloseCallbacks (
    MakeCallback (&Hotstuff::HandlePeerClose, this),
    MakeCallback (&Hotstuff::HandlePeerError, this));

}


//StopApplication
void Hotstuff::StopApplication()
{
  NS_LOG_FUNCTION (this);
  while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  if (m_socket_Rx) 
    {
      m_socket_Rx->Close ();
      m_socket_Rx->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket_Tx) 
    {
      m_socket_Tx->Close ();
    }
}

//Sending (multicast - send)
void Hotstuff::SendEvent(Ptr <Packet> packet)
{
   NS_LOG_FUNCTION (this);
   NS_LOG_INFO("Broadcasting packet");
   //create socket to send
         if (!m_socket_Tx)
            {
              m_socket_Tx = Socket::CreateSocket (GetNode (), m_tid);
                  if (m_socket_Tx->Bind () == -1)
                    {
                      NS_FATAL_ERROR ("Failed to bind socket");
                    }//if not bound
             }//if not socket
   
   m_socket_Tx->Connect(InetSocketAddress (Ipv4Address ("255.255.255.255"), InetSocketAddress::ConvertFrom (m_local).GetPort ())); //Broadcasting address
   m_socket_Tx->SetAllowBroadcast (true);
   m_socket_Tx->ShutdownRecv();
  
  //Is connection successful
      m_socket_Tx->SetConnectCallback (
        MakeCallback (&Hotstuff::ConnectionSucceeded, this),
        MakeCallback (&Hotstuff::ConnectionFailed, this));
  //Send the packet on the port
  m_txTrace (packet);
  m_socket_Tx->Send(packet);
  Address localAddress;
  m_socket_Tx->GetSockName (localAddress);
  m_txTraceWithAddresses(packet, localAddress, InetSocketAddress (Ipv4Address ("255.255.255.255"), InetSocketAddress::ConvertFrom (m_local).GetPort ()));
}

//Sending - ConnectionSucceeded
void Hotstuff::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

//Sending - ConnectionFailed
void Hotstuff::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

//Listening - HandleAccept
void Hotstuff::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&Hotstuff::HandleRead, this));
  m_socketList.push_back (s);
}
//Listening - HandlePeerClose
void Hotstuff::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
//Listening - HandlePeerError
void Hotstuff::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 

//Listening - HandleRead
void Hotstuff::HandleRead (Ptr<Socket> socket)
{
NS_LOG_FUNCTION(this<<socket);
Ptr<Packet> packet;
Address from;
Address localAddress;
while ((packet = socket->RecvFrom(from)))
        {
             socket->GetSockName (localAddress);
             m_rxTrace (packet);
             m_rxTraceWithAddressess (packet, from, localAddress);
             if(GetNode()->GetId()!=0)
                   HandlePacket (packet, from);
             else{
                     AddCnt();
                     if(GetCnt() == (2*m_faults) + 1){
                             std::ostringstream msg; 
                             msg << "2,Hello!" << '\0';
                             Ptr<Packet> packet1 = Create<Packet> ((uint8_t*) msg.str().c_str(), msg.str().length());
                             SendEvent(packet1);  
                      }// quorum is full        
             }//else
        }//while
}//Handle read


//Special function for non leaders
void Hotstuff::HandlePacket (Ptr<Packet> packet, Address add)
{
// Convert Packet to string
        uint8_t *buffer = new uint8_t[packet->GetSize ()];
        packet->CopyData(buffer, packet->GetSize ());
        std::string Value_recvd = std::string((char*)buffer);
//Check what strung it is
        char precur = Value_recvd[0];
    if(precur=='0'){
    	   std::ostringstream msg; 
           msg << "1,Hello!" << '\0';
           Ptr<Packet> packet1 = Create<Packet> ((uint8_t*) msg.str().c_str(), msg.str().length());
           socket->SendTo (packet, 0, add);        
    }
    if(precur=='2'){
        NS_LOG_INFO("Node " << GetNode()->GetId() << " is commiting to value");
        StopApplication();
    }      
}

}// Namespace
