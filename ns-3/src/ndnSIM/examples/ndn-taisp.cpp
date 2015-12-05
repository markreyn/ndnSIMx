// ndn-taisp.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#include "ndn-taisp/taisp-strategy.hpp"

using ns3::ndn::StackHelper;
using ns3::ndn::AppHelper;
using ns3::ndn::GlobalRoutingHelper;
using ns3::ndn::StrategyChoiceHelper;
using ns3::AnnotatedTopologyReader;

namespace ns3 {

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/topology-taisp.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  StackHelper ndnHelper;
  ndnHelper.InstallAll();

  // Installing global routing interface on all nodes
  GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // consumers/producers
  Ptr<Node> producer1 = Names::Find<Node>("Server1");
  Ptr<Node> producer2 = Names::Find<Node>("Server2");

  NodeContainer consumerNodes;
  Ptr<Node> consumer1 = Names::Find<Node>("YouTube");
  Ptr<Node> consumer2 = Names::Find<Node>("VoIP");
  Ptr<Node> consumer3 = Names::Find<Node>("FTP");
  Ptr<Node> consumer4 = Names::Find<Node>("Random");
  consumerNodes.Add(consumer1);
  consumerNodes.Add(consumer2);
  consumerNodes.Add(consumer3);
  consumerNodes.Add(consumer4);

  // Install NDN applications
  std::string prefix = "/taisp/taisp-experiment";

  // Install taisp strategy in node AC
  Ptr<Node> theac = Names::Find<Node>("AC");
  StrategyChoiceHelper::Install<nfd::fw::TaispStrategy>(theac, prefix);

  AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Frequency", StringValue("100"));
  consumerHelper.Install(consumerNodes);

  AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix(prefix);
  producerHelper.SetAttribute("PayloadSize", StringValue("4096"));
  producerHelper.Install(producer1);
  producerHelper.Install(producer2);

  // Add /prefix origins to ndn::GlobalRouter
  ndnGlobalRoutingHelper.AddOrigins(prefix, producer1);
  ndnGlobalRoutingHelper.AddOrigins(prefix, producer2);

  // Calculate and install FIBs
  GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(90.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int main(int argc, char *argv[])
{
  return ns3::main(argc, argv);
}
