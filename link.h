/*
 * Adithya Bhat <bhat24@purdue.edu>
 * 2019
 * */

#include "ns3/internet-module.h"
#include "ns3/object.h"

#include <vector>


using namespace ns3;

#ifndef __LINK__
#define __LINK__
class Link : public Object {
public:
	Link();
	std :: list < Ipv4Address > sinks;
	Ipv4Address source;
	void Send ( Ptr <Packet> p );
	Ptr<Node> node; // source node pointer
private:
	std :: list < Ipv4Address > :: iterator it;
};
#endif
