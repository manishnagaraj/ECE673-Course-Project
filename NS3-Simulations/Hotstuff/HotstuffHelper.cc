/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 
 * Hotstuff Algorithm - Helper Code
 * Needs to be linked with application
 * With the help of Hotstuff 
 *
 * Provides application for simulator
 *
 * Manish Nagaraj <mnagara@purdue.edu>
 * ECE, Purdue University
 * 2020
 * 
 */

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/application.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/packet-socket-address.h"
#include "ns3/string.h"
#include "ns3/data-rate.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

#include "HotstuffHelper.h"
#include "Hotstuff.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("HotstuffHelper");

HotstuffHelper :: HotstuffHelper(std::string protocol, Address address, uint64_t faults)
{
        NS_LOG_FUNCTION(this);
        m_factory.SetTypeId ("ns3::Hotstuff");
        m_factory.Set ("Protocol", StringValue (protocol));
        m_factory.Set ("Local", AddressValue (address)); 
        m_factory.Set ("NumberFaults", UintegerValue (faults)); 
        //Connect it to the Hotstuff application code
}

//define Install (function overload)
ApplicationContainer
HotstuffHelper :: Install (Ptr<Node> node) const
{
        NS_LOG_FUNCTION(this);
        return ApplicationContainer (InstallPriv (node));
}



ApplicationContainer 
HotstuffHelper :: Install (std::string nodeName) const
{
        NS_LOG_FUNCTION(this);
        Ptr<Node> node = Names::Find<Node> (nodeName);
        return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
HotstuffHelper :: Install (NodeContainer c) const
{
        NS_LOG_FUNCTION(this);
        ApplicationContainer apps;
        for(NodeContainer::Iterator i = c.Begin(); i!= c.End(); i++)
        {
                apps.Add(InstallPriv (*i));
        }
        return apps;
}

//Define what InstallPriv is

Ptr<Application>
HotstuffHelper :: InstallPriv (Ptr<Node> node) const
{
        NS_LOG_FUNCTION(this);
        Ptr<Application> app = m_factory.Create<Hotstuff> ();
        node->AddApplication (app);
        return app;
}
