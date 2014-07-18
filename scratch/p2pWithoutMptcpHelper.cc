#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("1p2pWithHelper1");

int
main(int argc, char *argv[])
{
  // Add some logging
  LogComponentEnable("1p2pWithHelper1", LOG_ALL);
  LogComponentEnable("MpTcpSocketBase", LOG_INFO);
  LogComponentEnable("MpTcpBulkSendApplication", LOG_ALL);

  NS_LOG_INFO("Create nodes");
  NodeContainer c;
  c.Create(2);

  // Topology construction
  NodeContainer n0n1 = NodeContainer(c.Get(0), c.Get(1));

  NS_LOG_INFO("Install network stack");
  InternetStackHelper netStack;
  netStack.Install(c);

  NS_LOG_INFO("Create channel");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("0.5ms"));
  NetDeviceContainer d0d1 = p2p.Install(n0n1);

  NS_LOG_INFO("Assign IP addresses");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.1.0", "255.255.255.0");
  ipv4.Assign(d0d1);

  NS_LOG_INFO("Install Routing tables");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  NS_LOG_INFO("Create Applications");

  // MPTCP SINK
    uint32_t servPort = 5000;
    Ptr<MpTcpPacketSink> sinkSocket = CreateObject<MpTcpPacketSink>();
    sinkSocket->SetAttribute("Local", AddressValue(InetSocketAddress(Ipv4Address::GetAny(), servPort)));
    Ptr<Node> server = c.Get(1);
    server->AddApplication(sinkSocket);
    ApplicationContainer Apps;
    Apps.Add(sinkSocket);
    Apps.Start(Seconds(0.0));
    Apps.Stop(Seconds(300));

  //  MPTCP SOURCE
  Ptr<MpTcpBulkSendApplication> src = CreateObject<MpTcpBulkSendApplication>();
  src->SetAttribute("Remote", AddressValue(Address(InetSocketAddress(Ipv4Address("10.0.1.2"), servPort))));
  src->SetAttribute("MaxBytes", UintegerValue(10000000));
  src->SetAttribute("SendSize", UintegerValue(10000000));
  src->SetBuffer(10000000);
  Ptr<Node> client = c.Get(0);
  client->AddApplication(src);
  ApplicationContainer pSource;
  pSource.Add(src);
  pSource.Start(Seconds(0.0));
  pSource.Stop(Seconds(300));

  NS_LOG_INFO("Simulation run");
  Simulator::Run();
  Simulator::Stop(Seconds(1000));
  Simulator::Destroy();
  NS_LOG_INFO("Simulation End");
}