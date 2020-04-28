/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 
 * This is a first trial of simulations of SMR protocols 
 * using NS3
 *
 * Zyzzyva Algorithm - Simulator Code
 * Needs to be linked with application
 * With the help of Zyzzyva-helper and Zyzzyva 
 *
 * Manish Nagaraj <mnagara@purdue.edu>
 * ECE, Purdue University
 * 2020
 * 
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/packet.h"

#include "ZyzzyvaHelper.h"
#include "Zyzzyva.h"

// Example of the sending of a datagram to a broadcast address
 //
 // Network topology
 //     =====================
 //       |     |    |    |
 //       n0    n1   n2   n3
 //
 //   n0 originates UDP broadcast to 255.255.255.255/discard port, which 
 //   is replicated and received on both n1 and n2

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Zyzzyva-Simulator");

int main(int argc, char *argv[])
{
	Time::SetResolution (Time::NS);
	
	bool verbose = true;
	uint32_t num_nodes = 10;
        uint64_t num_faults = (num_nodes - 1)/3;

	CommandLine cmd;
	cmd.AddValue ("Nodes", "Number of nodes", num_nodes);
	cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
	
	cmd.Parse (argc, argv);
	if(verbose)
	{
		LogComponentEnable("Zyzzyva", LOG_LEVEL_INFO);
		LogComponentEnable("ZyzzyvaHelper", LOG_LEVEL_INFO);
		LogComponentEnable("Zyzzyva-Simulator", LOG_LEVEL_INFO);
	}

	//Create Nodes
	NS_LOG_INFO("Creating Nodes");
	NodeContainer nodes;
	nodes.Create(num_nodes);

	//Create Channels
	NS_LOG_INFO("Creating channels");
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));// Attributes go here
	csma.SetChannelAttribute ("Delay", StringValue ("2ms"));

	//Install Csma Interfaces
	NetDeviceContainer devices = csma.Install(nodes);
	NS_LOG_INFO("Setting up Interfaces");
	InternetStackHelper internet;
	internet.Install ( nodes );
        // Setup Addresses for nodes 
	NS_LOG_INFO("Setup Address");
	Ipv4AddressHelper address;
	address.SetBase ("100.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = address. Assign ( devices );
	NS_LOG_INFO("Finished Address Assignment");

	// Adding Mobility 
	// Initializing mobility
	MobilityHelper mobility;
	//Change grid format
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
	  "MinX", DoubleValue (0.0),
	  "MinY", DoubleValue (0.0),
	  "DeltaX", DoubleValue (5.0),
	  "DeltaY", DoubleValue (10.0),
	  "GridWidth", UintegerValue (5),
	  "LayoutType", StringValue ("ColumnFirst")); 
	// Or you could do RowFirst or ColumnFirst

	//Set Mobility of AP
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (nodes);

	// Set Mobility of Stations can be either ConstantPositionMobilityModel or RandomWalk2dMobilityModel
	//mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));



	//Install Application
	uint16_t port = 9;
	NS_LOG_INFO ("Create Applications.");
	ZyzzyvaHelper Zyzzyva ("ns3::UdpSocketFactory", 
		              Address (InetSocketAddress (Ipv4Address::GetAny (), port)), num_faults);
	 
	ApplicationContainer app = Zyzzyva.Install (nodes);
	// Start the application
	app.Start (Seconds (1.0));
	app.Stop (Seconds (10.0));
        //AsciiTraceHelper ascii;
        //csma.EnableAsciiAll (ascii.CreateFileStream ("myfirst.tr"));
	csma.EnablePcapAll ("csma-broadcast", false);
        Packet::EnablePrinting ();
        Packet::EnableChecking ();
        Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
	NS_LOG_INFO ("Run Simulation.");
	Simulator::Run ();
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");

}

