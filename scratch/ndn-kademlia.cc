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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>

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
main (int argc, char *argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
  Config::SetDefault ("ns3::QueueBase::MaxSize", StringValue ("20p"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf
  // --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  std::vector<std::string> array = {
      "e42763c64661f37edc4d45887e6c857699507646", "c2a17f016c70062b210f4dca7e4ea61db7d8f0ac",
      "f4eb250cf7dc4b615d110b54e54ebd4018ec232e", "637a81ed8e8217bb01c15c67c39b43b0ab4e20f1",
      "fbc12b721be870d2a7c35f85eefa8176116f3f75", "7d61c58df413e74b615edf17891b72e8b9897559",
      "3401e667e3b21395fe99dded1cdeddd20bb66a84", "ff43122edae8e7e3a2b7d8d8bdc3891dcb780e2f",
      "63a8286d1760cfc39c38cfe3468eed76ffe500eb", "d2bd04146bf0890619217623c65d917c6914a51d",
      "bfb7759a67daeb65410490b4d98bb9da7d1ea2ce"};

  /**
  uint32_t nodeCount = 3;
  clx::sha1 hash;
  for (uint32_t i = 0; i < nodeCount; i++)
    {
      const uuid id = random_generator () ();
      std::string contentHash = hash.encode (boost::lexical_cast<std::string> (id)).to_string ();
      array.push_back (contentHash);
    }

  */

  for (vector<std::string>::iterator i = array.begin (); i != array.end (); i++)
    {
      std::cout << "[Node ID Seed] Node " << distance (array.begin (), i) << " : " << *i << endl;
    }

  // Creating nodes
  NodeContainer nodes;
  nodes.Create (array);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install (nodes.Get (0), nodes.Get (1));
  p2p.Install (nodes.Get (1), nodes.Get (4));
  p2p.Install (nodes.Get (2), nodes.Get (3));
  p2p.Install (nodes.Get (3), nodes.Get (4));
  p2p.Install (nodes.Get (4), nodes.Get (5));
  p2p.Install (nodes.Get (5), nodes.Get (6));
  p2p.Install (nodes.Get (6), nodes.Get (7));
  p2p.Install (nodes.Get (6), nodes.Get (9));
  p2p.Install (nodes.Get (7), nodes.Get (8));
  p2p.Install (nodes.Get (9), nodes.Get (10));

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.InstallAll ();

  // Installing global routing interface on all nodes
  GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll ("/", "/localhost/nfd/strategy/kondn/%FD%05");

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper.SetPrefix ("/nakazato.lab/testing");
  consumerHelper.SetAttribute ("Frequency",
                               StringValue ("3")); // 10 interests a second
  auto apps = consumerHelper.Install (nodes.Get (0)); // first node
  apps.Stop (Seconds (10.0)); // stop the consumer app at 10 seconds mark

  ndn::AppHelper chaserHelper ("ns3::ndn::ConsumerCbr");
  // Consumer will request /prefix/0, /prefix/1, ...
  chaserHelper.SetPrefix ("/nakazato.lab/testing");
  chaserHelper.SetAttribute ("Frequency",
                             StringValue ("2")); // 10 interests a second
  auto chaser = chaserHelper.Install (nodes.Get (10));
  chaser.Stop (Seconds (10.0));

  // Producer
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  ndnGlobalRoutingHelper.AddOrigins ("/nakazato.lab/testing", nodes.Get (2));
  producerHelper.SetPrefix ("/nakazato.lab/testing");
  producerHelper.SetAttribute ("PayloadSize", StringValue ("1024"));
  producerHelper.Install (nodes.Get (2)); // last node

  // Add Routes to Node
  for (const auto &e : array | boost::adaptors::indexed ())
    {
      auto index = e.index ();
      std::string value = e.value ();
      ndnGlobalRoutingHelper.AddOrigins ("/" + value, nodes.Get (index));
    }

  // Calculate and install FIBs
  GlobalRoutingHelper::CalculateRoutes ();

  Simulator::Stop (Seconds (5.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

} // namespace ns3

int
main (int argc, char *argv[])
{
  return ns3::main (argc, argv);
}