/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Second Try at this :p
 *
 * So far just combined onoffapplication and packetsink 
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

#include "PBFT.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("PBFT");

NS_OBJECT_ENSURE_REGISTERED (PBFT);

//TID definition
TypeId 
PBFT::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PBFT")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<PBFT> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&PBFT::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&PBFT::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("NumberFaults", "The number of faulty nodes in the system",
                   UintegerValue (1),
                   MakeUintegerAccessor (&PBFT::m_faults),
                   MakeUintegerChecker<uint64_t> (1))
    .AddAttribute ("Local",
                   "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&PBFT::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol",
                   "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&PBFT::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Rx",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&PBFT::m_rxTrace),
                     "ns3::Packet::AddressTracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&PBFT::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&PBFT::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&PBFT::m_txTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

//Constructor
PBFT::PBFT ()
{
  NS_LOG_FUNCTION (this);
  m_socket_Rx = 0;
  m_socket_Tx = 0;
  m_prop_cnt = 0;
  m_pre_cnt = 0;
  m_com_cnt = 0;
}

//~constructor
PBFT::~PBFT()
{
  NS_LOG_FUNCTION (this);
}

//Get listening socket
Ptr<Socket>
PBFT::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket_Rx;
}

//Get transmit socket
Ptr<Socket>
PBFT::GetSendingSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket_Tx;
}

//Get list of accepted sockets
std::list<Ptr<Socket> >
PBFT::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

//Get counts
uint64_t PBFT::GetPropCnt () 
{
  NS_LOG_FUNCTION (this);
  return m_prop_cnt;
}

uint64_t PBFT::GetPreCnt ()
{
  NS_LOG_FUNCTION (this);
  return m_pre_cnt;
}

uint64_t PBFT::GetComCnt () 
{
  NS_LOG_FUNCTION (this);
  return m_com_cnt;
}

//Increment counts
void PBFT::AddPropCnt (void) 
{
  NS_LOG_FUNCTION (this);
  m_prop_cnt++;
}

void PBFT::AddPreCnt (void) 
{
  NS_LOG_FUNCTION (this);
  m_pre_cnt++;
}

void PBFT::AddComCnt (void) 
{
  NS_LOG_FUNCTION (this);
  m_com_cnt++;
}

//DoDispose
void PBFT::DoDispose (void)
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
void PBFT::StartApplication()
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
  m_socket_Rx->SetRecvCallback (MakeCallback (&PBFT::HandleRead, this));
  m_socket_Rx->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&PBFT::HandleAccept, this));
  m_socket_Rx->SetCloseCallbacks (
    MakeCallback (&PBFT::HandlePeerClose, this),
    MakeCallback (&PBFT::HandlePeerError, this));

}

//StopApplication
void PBFT::StopApplication()
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

//Sending 
void PBFT::SendEvent(Ptr <Packet> packet)
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
        MakeCallback (&PBFT::ConnectionSucceeded, this),
        MakeCallback (&PBFT::ConnectionFailed, this));
  //Send the packet on the port
  m_txTrace (packet);
  m_socket_Tx->Send(packet);
  Address localAddress;
  m_socket_Tx->GetSockName (localAddress);
  m_txTraceWithAddresses(packet, localAddress, InetSocketAddress (Ipv4Address ("255.255.255.255"), InetSocketAddress::ConvertFrom (m_local).GetPort ()));
}

//Sending - ConnectionSucceeded
void PBFT::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

//Sending - ConnectionFailed
void PBFT::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

//Listening - HandleAccept
void PBFT::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&PBFT::HandleRead, this));
  m_socketList.push_back (s);
}
//Listening - HandlePeerClose
void PBFT::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
//Listening - HandlePeerError
void PBFT::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
//Listening - HandleRead
void PBFT::HandleRead (Ptr<Socket> socket)
{
 NS_LOG_FUNCTION(this << socket);
 Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0)
        { //EOF
          break;
        }
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet received "
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ());
        }
      socket->GetSockName (localAddress);
      m_rxTrace (packet, from);
      m_rxTraceWithAddresses (packet, from, localAddress);
      HandlePacket(packet);
    }
}

//Handling Packet - Deciding to forward or not
void PBFT:: HandlePacket (Ptr<Packet> packet)
{
// Convert Packet to string
        uint8_t *buffer = new uint8_t[packet->GetSize ()];
        packet->CopyData(buffer, packet->GetSize ());
        std::string Value_recvd = std::string((char*)buffer);
//Check what strung it is
        char precur = Value_recvd[0];
//Three cases 
//Initial proposal (pre-prepare in paper) - send prepare
//Received 3 prepairs send commit to all
//Received 3 commits actually commit

if(precur=='0'){
//Send prepare.
        std::ostringstream msg; 
        msg << "1,Hello!" << '\0';
        Ptr<Packet> packet1 = Create<Packet> ((uint8_t*) msg.str().c_str(), msg.str().length());
        SendEvent(packet1);  
}//pre-prepare stage

if(precur=='1'){
	AddPreCnt();
	if(GetPreCnt()== m_faults + 1){
                NS_LOG_INFO("Node " << GetNode()->GetId() << " is preparing");
                std::ostringstream msg; 
		msg << "2,Hello!" << '\0';
		Ptr<Packet> packet1 = Create<Packet> ((uint8_t*) msg.str().c_str(), msg.str().length());
		SendEvent(packet1);  		
	}//else if
}//prepare stage

if(precur=='2'){
        AddComCnt();
	if(GetComCnt()== (2*m_faults) + 1)
		NS_LOG_INFO("Node " << GetNode()->GetId() << " is commiting to value");
//check commit count 
}//commit stage 

}

}// namespace ns3

