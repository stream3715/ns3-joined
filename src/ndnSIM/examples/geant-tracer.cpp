
#include "ns3/core-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3 {

int
main(int argc, char* argv[])
{

  // Setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/geant.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.InstallAll();

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> producerMain = Names::Find<Node>("rtr-35");
  Ptr<Node> producerNoise1 = Names::Find<Node>("rtr-1");
  Ptr<Node> producerNoise2 = Names::Find<Node>("rtr-36");

  NodeContainer mainConsumerNodes;
  mainConsumerNodes.Add(Names::Find<Node>("rtr-17"));

  NodeContainer noiseConsumerNodes1;
  noiseConsumerNodes1.Add(Names::Find<Node>("rtr-12"));

  NodeContainer noiseConsumerNodes2;
  noiseConsumerNodes2.Add(Names::Find<Node>("rtr-31"));

  NodeContainer subConsumerNodes;

  // subConsumerNodes.Add(Names::Find<Node>("rtr-1"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-2"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-3"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-4"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-5"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-6"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-7"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-8"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-9"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-10"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-11"));
  // subConsumerNodes.Add(Names::Find<Node>("rtr-12"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-13"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-14"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-15"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-16"));
  // subConsumerNodes.Add(Names::Find<Node>("rtr-17"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-18"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-19"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-20"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-21"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-22"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-23"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-24"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-25"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-26"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-27"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-28"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-29"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-30"));
  // subConsumerNodes.Add(Names::Find<Node>("rtr-31"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-32"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-33"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-34"));
  // subConsumerNodes.Add(Names::Find<Node>("rtr-35"));
  // subConsumerNodes.Add(Names::Find<Node>("rtr-36"));
  subConsumerNodes.Add(Names::Find<Node>("rtr-37"));

  // Install NDN applications
  std::string prefix = "/prefix";
  std::string noise1 = "/noise1";
  std::string noise2 = "/noise2";

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Frequency", StringValue("100")); // 100 interests a second
  consumerHelper.Install(mainConsumerNodes);

  consumerHelper.SetPrefix(noise1);
  consumerHelper.Install(noiseConsumerNodes1);

  consumerHelper.SetPrefix(noise2);
  consumerHelper.Install(noiseConsumerNodes2);

  ndn::AppHelper subConsumerHelper("ns3::ndn::ConsumerCbr");
  subConsumerHelper.SetPrefix(prefix);
  subConsumerHelper.SetAttribute("Frequency", StringValue("100"));
  subConsumerHelper.Install(subConsumerNodes);

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix(prefix);
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(producerMain);

  producerHelper.SetPrefix(noise1);
  producerHelper.Install(producerNoise1);

  producerHelper.SetPrefix(noise2);
  producerHelper.Install(producerNoise2);

  ndnGlobalRoutingHelper.AddOrigins(prefix, producerMain);
  ndnGlobalRoutingHelper.AddOrigins(noise1, producerNoise1);
  ndnGlobalRoutingHelper.AddOrigins(noise2, producerNoise2);

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(10.0));

  ndn::AppDelayTracer::InstallAll("app-delays-trace.txt");

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
