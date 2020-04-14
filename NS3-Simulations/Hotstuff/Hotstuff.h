/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 *
 *
 * Author:  Manish Nagaraj (mnagara@purdue.edu)
 */

#ifndef Hotstuff_H
#define Hotstuff_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/data-rate.h"

namespace ns3 {

class Address;
class Socket;
class Packet;
class RandomVariableStream;

class Hotstuff : public Application 
{
public:
  static TypeId GetTypeId (void);
  Hotstuff ();
  virtual ~Hotstuff ();
  uint64_t GetCnt ();
  uint64_t        m_quorum;      //!< Total number of signatures in quorum
  Ptr<Socket> GetListeningSocket (void) const;
  Ptr<Socket> GetSendingSocket (void) const;
  std::list<Ptr<Socket> > GetAcceptedSockets (void) const;
 
protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  void AddCnt (void);
  void SendEvent (Ptr<Packet> packet);
  void ConnectionSucceeded(Ptr<Socket> socket);
  void ConnectionFailed(Ptr<Socket> socket);
  void HandleRead (Ptr<Socket> socket);
  void HandlePacket (Ptr<Packet> packet, Address add);
  void HandleAccept (Ptr<Socket> socket, const Address& from);
  void HandlePeerClose (Ptr<Socket> socket);
  void HandlePeerError (Ptr<Socket> socket);


  Ptr<Socket>     m_socket_Rx;       //!< Listening socket
  Ptr<Socket>     m_socket_Tx;       //!< Sending socket
  std::list<Ptr<Socket> > m_socketList; //!< the accepted sockets

  bool            m_connected;    //!< True if connected
  Address         m_local;        //!< Local address to bind to
  uint32_t        m_pktSize;      //!< Size of packets
  uint64_t        m_faults;      //!< Number of faults
  TypeId          m_tid;          //!< Protocol TypeId
  DataRate        m_cbrRate;      //!< Rate that data is generated

  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;
  TracedCallback<Ptr<const Packet> > m_txTrace;
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_txTraceWithAddresses;

};

} // namespace ns3

#endif /* Hotstuff_H */

