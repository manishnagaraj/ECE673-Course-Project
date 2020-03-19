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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"

#include "PBFTHelper.h"
#include "PBFT.h"

#define LOG(str) NS_LOG_INFO(str)

// Default Network Topology
//
//   Wifi 10.1.3.0
//  AP              
//  *    *    *    *
//  |    |    |    |   
// n0   n1   n2   n3 
//


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("PBFT-Simulator");


//AddNextNodes ( &sinks, nodes , Start , N );
void AddNextNodes( NodeContainer *sinks, NodeContainer nodes, uint64_t start , uint64_t N ) 
{
	start = start % N; // In case, start is already out of bounds
	for ( uint64_t i = 0 ; i < N ; i++ ) {
		uint64_t idx = ( start + i ) % N ;
		// Insert idx-th node to the link here
		sinks->Add ( nodes. Get(idx) );
	}
}

//AddNextAddress ( &link->sinks , sinks [N] );
void AddNextAddress ( std :: list < Ipv4Address> *addr_list, NodeContainer sink ) {
	size_t N = sink.GetN();
	printf ( "Address: " ); 
	for ( size_t i = 0 ; i < N ; i++ ) {
		Ptr<Node> node = sink .Get(i);
		Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
		Ipv4Address addr = ipv4->GetAddress(1, 0).GetLocal();
		std :: cout << addr;
		std :: cout << ", ";
		addr_list-> push_back ( addr );
	}
	printf ( "\n" );
}



int 
main (int argc, char *argv[])
{

        Time::SetResolution (Time::NS);

	bool verbose = true;
	uint32_t nCsma = 4;

	CommandLine cmd;
	cmd.AddValue ("nWifi", "Number of wifi STA devices", nCsma);
	cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

	cmd.Parse (argc, argv);

	if(verbose)
	 {
	   LogComponentEnable("PBFT", LOG_LEVEL_INFO);
	   LogComponentEnable("PBFT-Helper", LOG_LEVEL_INFO);
	   LogComponentEnable("PBFT-Simulator", LOG_LEVEL_INFO);
	 }

	// Create nodes;
	LOG("Create Nodes");
	NodeContainer nodes;
	nodes.Create (nCsma);
        //Keep track of multicasting
        NodeContainer sinks[nCsma * nCsma];
        for ( uint64_t i = 0 ; i < N ; i ++ ) {
		for ( uint64_t j = 0 ; j < N ; j++ ) {
			Ptr<Node> source = nodes.Get ( i );
			AddNextNodes ( &sinks [ (i*N)+j ], nodes , i+j+1 , N );
		}
	}
        

       // Create channels
	LOG("Create Channel");
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));// BLE Attributes go here
	csma.SetChannelAttribute ("Delay", StringValue ("2ms"));

        // Install Csma Interfaces
        NetDeviceContainer devices = csma.Install (nodes);
	LOG("Setup Interfaces");
        // Setup Protocol Stack
	InternetStackHelper internet;
	internet.Install ( nodes );
        // Setup Addresses for nodes 
	LOG("Setup Address");
	Ipv4AddressHelper address;
	address.SetBase ("100.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = address. Assign ( devices );
	LOG("Finished Address Assignment");

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

	// Design the application
	// Install It
 	PBFTHelper helper;
	ApplicationContainer apps = helper.Install ( nodes ); 


        //Keep Track of addresses and multi casting links
	for ( uint64_t i = 0 ; i < N ; i++ ) {
		for ( uint64_t j = 0 ; j < D_OUT ; j++ ) {
			Link *link = new Link();
			link->node = nodes. Get (i);
			link->source = interfaces.GetAddress (i,0);
			AddNextAddress ( &link->sinks , sinks [ (i*D_OUT)+j ] );
			Ptr<Application> sender = apps.Get(i);
			Ptr<PBFT> bs = sender->GetObject<PBFT>();
			bs->outLinks .push_back (link);
		}
	}
       
	//Establish Routing tables

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	//Tell Simulator a time to stop
	Simulator::Stop (Seconds (10.0));

	//Get Tracing 
	   csma.EnablePcap ("PBFT-Simulator", devices.Get (0));

	//Run the simulator
	  Simulator::Run ();
	  Simulator::Destroy ();
	  return 0;
}
