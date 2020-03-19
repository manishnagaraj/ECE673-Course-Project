/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 
 * PBFT Helper .h file
 *
 * Manish Nagaraj <mnagara@purdue.edu>
 * ECE, Purdue University
 * 2020
 * 
 */

#include "ns3/core-module.h"
#include "ns3/application.h"
#include "ns3/application-container.h"
#include "ns3/node-container.h"

using namespace ns3;

class PBFTHelper
{
public: 
	PBFTHelper();
	ApplicationContainer Install (Ptr<Node> node) const;
	ApplicationContainer Install (std::string nodeName) const;
	ApplicationContainer Install (NodeContainer c) const;
private:
	Ptr<Application> InstallPriv (Ptr<Node> node) const;
	ObjectFactory m_factory; //!< Object factory.
};
