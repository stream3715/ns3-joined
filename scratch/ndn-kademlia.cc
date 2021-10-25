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

  std::vector<std::string> array = {"c42763c64661f37edc4d45887e6c857699507646",
                                    "e2a17f016c70062b210f4dca7e4ea61db7d8f0ac",
                                    "f4eb250cf7dc4b615d110b54e54ebd4018ec232e"};

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
  p2p.Install (nodes.Get (1), nodes.Get (2));

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
                               StringValue ("2")); // 10 interests a second
  auto apps = consumerHelper.Install (nodes.Get (0)); // first node
  apps.Stop (Seconds (10.0)); // stop the consumer app at 10 seconds mark

  // Producer
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
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

  Simulator::Stop (Seconds (1.0));

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