/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 
 * PBFT Algorithm - Helper Code
 * Needs to be linked with application
 * With the help of PBFT 
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

#include "PBFTHelper.h"
#include "PBFT.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("PBFTHelper");

PBFTHelper :: PBFTHelper()
{
        NS_LOG_FUNCTION(this);
        m_factory.SetTypeId (PBFT :: GetTypeId ()); 
        //Connect it to the PBFT application code
}

//define Install (function overload)
ApplicationContainer
PBFTHelper :: Install (Ptr<Node> node) const
{
        NS_LOG_FUNCTION(this);
        return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer 
PBFTHelper :: Install (std::string nodeName) const
{
        NS_LOG_FUNCTION(this);
        Ptr<Node> node = Names::Find<Node> (nodeName);
        return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
PBFTHelper :: Install (NodeContainer c) const
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
PBFTHelper :: InstallPriv (Ptr<Node> node) const
{
        NS_LOG_FUNCTION(this);
        Ptr<Application> app = m_factory.Create<PBFT> ();
        node->AddApplication (app);
        return app;
}
