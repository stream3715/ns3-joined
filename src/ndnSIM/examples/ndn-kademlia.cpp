/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors
 *and contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the
 *terms of the GNU General Public License as published by the Free Software
 *Foundation, either version 3 of the License, or (at your option) any later
 *version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY
 *WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 *A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see
 *<http://www.gnu.org/licenses/>.
 **/

// ndn-kademlia.cpp

#include "ns3/core-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace ns3 {

/**
 * This scenario simulates a very kademlia network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-kademlia
 */

using ns3::ndn::GlobalRoutingHelper;
using namespace boost::uuids;

int
main(int argc, char* argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

  std::string prefix = "/nakazato.lab/test";

  // Read optional command-line parameters (e.g., enable visualizer with ./waf
  // --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  std::vector<std::string> array =
    {"be43a63a0fa44ec48dd74e52ed24aa6b00000000", "6bc1933f452d4a3d80fca48b68dde16b00000000",
     "d07b8294aeb744c183214868eea93b8600000000", "76d4bf62b77e4eaba6ecc6154992141500000000",
     "a6cbb31c62eb49d2bd7fcafbb2352ba200000000", "9d5500bc83744a14a6cefd539a62682500000000",
     "2c52fefc6b514356a939ea011ecf729e00000000", "042d4ec998c64ddb9ab5c79aded054b800000000",
     "ef7220b17e654963a5d73a946a04c29300000000", "5aa2ae572ea64a9f8e734eedd95daf7900000000",
     "2ad927692dd84ad791c3cd49d007796600000000", "a569661bf9e24e9f99a7382f463f734a00000000",
     "7f3a03b5293e494bab51521fd635de6500000000", "cce3ea4eabfa435aac40bf0445481fce00000000",
     "fde7220b8a114bab9a13d083f47d511500000000", "49b2a424234d484eab5bbe59c7cd8d1000000000",
     "0e3792362b9d4743ab51d164c643a3f800000000", "dbdabc9bcced43249c18f3597c623ce100000000",
     "4ccebe825c5440d1addcb1ff745d56dc00000000", "5befe52ddf1e44c2993307c36773d12200000000",
     "701f2b7125a54819a89db57b29e590d800000000", "2a0582a043e6413a80fcd90f434655e200000000",
     "7204fbe3991845c895508dc9b812b0b200000000", "53243ad1ee784bf9b845c21caeb4cb6b00000000",
     "e84aa8169baf41b0b5a286fe3d8faf8600000000", "209526ddd8dd43828d8499c57027cccb00000000",
     "0fa8ff3b0fef4a3c9ee4b79748c32a7d00000000", "716eb67549b444668c88c368b510daac00000000",
     "4b03008850054b82ae882e37c099161900000000", "ad625073d761484790208fe9ca1c2f4a00000000",
     "27bb620ff3264c5c9c98d494ab3af52700000000", "b80d831ce7834cc3b48309f755b5a42e00000000",
     "fde7269a57b146019159e9ae684308d700000000", "313aade3dea04fefb4d59b045904d3d100000000",
     "976953572c8f4e8dbd433ff7be26db3c00000000", "eeeb96fddd864b7c9ef37fc80a6c510c00000000",
     "20f167f318fb4e59b0be12aa935ecaf200000000"};

  /**
  int len = array.size();
  uint32_t nodeCount = 3;
  clx::sha1 hash;
  for (uint32_t i = 0; i < nodeCount; i++)
    {
      const uuid id = random_generator () ();
      std::string contentHash = hash.encode (boost::lexical_cast<std::string> (id)).to_string ();
      array.push_back (contentHash);
    }

  */

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/geant.txt");
  ns3::NodeContainer nodes = topologyReader.Read(array);

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Installing global routing interface on all nodes
  GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/kondn/%FD%05");

  // Installing applications

  // Producer
  Ptr<Node> producerMain = Names::Find<Node>("rtr-5");

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetPrefix(prefix);
  producerHelper.Install(producerMain); // last node

  // ndnGlobalRoutingHelper.AddOrigins(prefix, producerMain);

  NodeContainer majorConsumerNodes;

  majorConsumerNodes.Add(Names::Find<Node>("rtr-1"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-2"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-3"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-4"));
  // majorConsumerNodes.Add(Names::Find<Node>("rtr-5"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-6"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-7"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-8"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-9"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-10"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-11"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-12"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-13"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-14"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-15"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-16"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-18"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-19"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-20"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-21"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-22"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-23"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-24"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-25"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-26"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-17"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-28"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-29"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-30"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-31"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-32"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-33"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-34"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-35"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-36"));
  majorConsumerNodes.Add(Names::Find<Node>("rtr-37"));

  ndn::AppHelper majorConsumerHelper("ns3::ndn::ConsumerZipfMandelbrot");
  majorConsumerHelper.SetPrefix(prefix + "/major");
  majorConsumerHelper.SetAttribute("Frequency", StringValue("100")); // 100 interests a second
  majorConsumerHelper.Install(majorConsumerNodes);

  NodeContainer minorConsumerNodes;
  minorConsumerNodes.Add(Names::Find<Node>("rtr-1"));
  minorConsumerNodes.Add(Names::Find<Node>("rtr-8"));
  minorConsumerNodes.Add(Names::Find<Node>("rtr-9"));

  ndn::AppHelper minorConsumerHelper("ns3::ndn::ConsumerZipfMandelbrot");
  minorConsumerHelper.SetPrefix(prefix + "/minor");
  minorConsumerHelper.SetAttribute("Frequency", StringValue("1"));
  minorConsumerHelper.Install(minorConsumerNodes);

  // Add Kademlia Based Routes to Node
  for (const auto& e : array | boost::adaptors::indexed()) {
    auto index = e.index();
    std::string value = e.value();
    ndnGlobalRoutingHelper.AddOrigins("/" + value, nodes.Get(index));
  }

  // Calculate and install FIBs
  GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(30.0));

  ndn::L3RateTracer::InstallAll("rate-trace.tsv", Seconds(0.5));
  L2RateTracer::InstallAll("drop-trace.tsv", Seconds(0.5));
  ndn::AppDelayTracer::InstallAll("app-delays-trace.tsv");

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
}
int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
