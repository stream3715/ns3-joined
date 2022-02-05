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

#include <clx/sha1.h>
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

  string prefix = "/nakazato.lab/test";

  // Read optional command-line parameters (e.g., enable visualizer with ./waf
  // --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  vector<string> array = {"be43a63a0fa44ec48dd74e52ed24aa6b", "6bc1933f452d4a3d80fca48b68dde16b",
                          "d07b8294aeb744c183214868eea93b86", "76d4bf62b77e4eaba6ecc61549921415",
                          "a6cbb31c62eb49d2bd7fcafbb2352ba2", "9d5500bc83744a14a6cefd539a626825",
                          /*
                          "2c52fefc6b514356a939ea011ecf729e", "042d4ec998c64ddb9ab5c79aded054b8",
                          "ef7220b17e654963a5d73a946a04c293", "5aa2ae572ea64a9f8e734eedd95daf79",
                          "2ad927692dd84ad791c3cd49d0077966", "a569661bf9e24e9f99a7382f463f734a",
                          "7f3a03b5293e494bab51521fd635de65", "cce3ea4eabfa435aac40bf0445481fce",
                          "fde7220b8a114bab9a13d083f47d5115", "49b2a424234d484eab5bbe59c7cd8d10",
                          "0e3792362b9d4743ab51d164c643a3f8", "dbdabc9bcced43249c18f3597c623ce1",
                          "4ccebe825c5440d1addcb1ff745d56dc", "5befe52ddf1e44c2993307c36773d122",
                          "701f2b7125a54819a89db57b29e590d8", "2a0582a043e6413a80fcd90f434655e2",
                          "7204fbe3991845c895508dc9b812b0b2", "53243ad1ee784bf9b845c21caeb4cb6b",
                          "e84aa8169baf41b0b5a286fe3d8faf86", "209526ddd8dd43828d8499c57027cccb",
                          "0fa8ff3b0fef4a3c9ee4b79748c32a7d", "716eb67549b444668c88c368b510daac",
                          "4b03008850054b82ae882e37c0991619", "ad625073d761484790208fe9ca1c2f4a",
                          "27bb620ff3264c5c9c98d494ab3af527", "b80d831ce7834cc3b48309f755b5a42e",
                          "fde7269a57b146019159e9ae684308d7", "313aade3dea04fefb4d59b045904d3d1",
                          "976953572c8f4e8dbd433ff7be26db3c", "eeeb96fddd864b7c9ef37fc80a6c510c",
                          "20f167f318fb4e59b0be12aa935ecaf2", "4769a7e54c524f6695bb0bbaeb50a1ad",
                          "643785263cf34d2387294a3710fbe894", "0ac79b126fa449ddbc11f4a6f2af1bd6",
                          "f98aaa6727374bbca4534c0180a3bbd0", "d93a3126a1534ad58c8c4530c5109103",
                          "0d853edc1cef4b24888a56a631118b48", "e2fbd20580a64dc5b8c58c0b9dae8f1f",
                          "7846abc1f4e2439b8eb24267f0f22cdf", "41ddf3004c8b40e2b9e0359801fe8db5",
                          "0c3c19f024e7451891bc69aedf5e28b6", "c00cc1201b234210a6ab50888d2e5194",
                          "f56c73213b1a4230877bedcb95f5e4d3", "86853bc65adb42caa75bfd08b466faa3",
                          "754b490f5ea049a88cce6d044f4c03df", "b3d090e085df49c4b060e85852b19c57",
                          "c10cfe1c737e49be99590426be4830b7", "1935780ba4d44fbd83afba68cc32ac33",
                          "d149d5a0fa564d9084eccb24f4979d5c", "bd1897961d4549858b1473c8eb13c7f1",
                          "e4314ec7f1c24c4a9b40d6e0e17154b9", "82fcf6ecce904cf4acebbae4cf78e82c",
                          "42327ee1d10f41b4bb43963bff1ea848", "ab511e45463540e592f9325f342ce738",
                          "86313825ecba490889cf7128ec4a6d50", "1c0ddbdf388f4554bf496ee70f023ab4",
                          "21d06f81b5864fb392961df5d841d291", "c85d31379aba4fea89cf2c40a55c196e",
                          "63883d8248ab4758809316c98441172a", "089d4ebe63be4f4bad5fc3d4399c3f46",
                          "13974e3ff208433a8c82c40a906e98c6", "3f3a2e177c834dd383dca7afd74020d7",
                          "6620b642857e48ab8f20c622ddceee79", "ad527423d3a54072abae030f2387308f",
                          "546bfdc9efc84e319416a20e55cb3539", "2b95e1568dd14fa89f94733a168d715b",
                          */
                          "17b9ba49c7a648d0a2b23bc2ebdb02e4", "fa880995a1854686b0ae54c712b3a83e"};

  cout << "Array size: " << array.size() << endl;
  /**
  uint32_t nodeCount = 3;
  clx::sha1 hash;
  for (uint32_t i = 0; i < nodeCount; i++)
    {
      const uuid id = random_generator () ();
      string contentHash = hash.encode (boost::lexical_cast<string>
  (id)).to_string (); array.push_back (contentHash);
    }

  */

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/geant_mini.txt");
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
  Ptr<Node> producerMain = Names::Find<Node>("leaf-2");

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetPrefix(prefix);
  producerHelper.Install(producerMain); // last node

  ndnGlobalRoutingHelper.AddOrigins(prefix, producerMain);

  NodeContainer mainConsumerNodes;
  mainConsumerNodes.Add(Names::Find<Node>("leaf-3"));

  /*
    NodeContainer subConsumerNodes;

    subConsumerNodes.Add(Names::Find<Node>("rtr-1"));
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
    subConsumerNodes.Add(Names::Find<Node>("rtr-12"));
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
    subConsumerNodes.Add(Names::Find<Node>("rtr-31"));
    subConsumerNodes.Add(Names::Find<Node>("rtr-32"));
    subConsumerNodes.Add(Names::Find<Node>("rtr-33"));
    subConsumerNodes.Add(Names::Find<Node>("rtr-34"));
    // subConsumerNodes.Add(Names::Find<Node>("rtr-35"));
    subConsumerNodes.Add(Names::Find<Node>("rtr-36"));
    subConsumerNodes.Add(Names::Find<Node>("rtr-37"));
    */

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerZipfMandelbrot");
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Frequency", StringValue("100")); // 100 interests a second
  consumerHelper.Install(mainConsumerNodes);

  /*
    ndn::AppHelper subConsumerHelper("ns3::ndn::ConsumerZipfMandelbrot");
    subConsumerHelper.SetPrefix(prefix);
    subConsumerHelper.SetAttribute("Frequency", StringValue("100"));
    subConsumerHelper.Install(subConsumerNodes);
  */

  // Add Kademlia Based Routes to Node
  for (const auto& e : array | boost::adaptors::indexed()) {
    auto index = e.index();
    string value = e.value();
    ndnGlobalRoutingHelper.AddOrigins("/" + value, nodes.Get(index));
  }

  // Calculate and install FIBs
  GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(1.0));

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
