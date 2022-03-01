/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne
 * University, Washington University in St. Louis, Beijing Institute of
 * Technology, The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kondn-strategy.hpp"

#include "algorithm.hpp"
#include "common/logger.hpp"

namespace nfd {
namespace fw {

NFD_LOG_INIT(KoNDNStrategy);
NFD_REGISTER_STRATEGY(KoNDNStrategy);

const time::milliseconds KoNDNStrategy::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds KoNDNStrategy::RETX_SUPPRESSION_MAX(250);

KoNDNStrategy::KoNDNStrategy(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , ProcessNackTraits(this)
  , m_retxSuppression(RETX_SUPPRESSION_INITIAL, RetxSuppressionExponential::DEFAULT_MULTIPLIER,
                      RETX_SUPPRESSION_MAX)
{
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    NDN_THROW(std::invalid_argument("KoNDNStrategy does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    NDN_THROW(std::invalid_argument("KoNDNStrategy does not support version "
                                    + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
  this->m_nodeId = forwarder.getNodeId();
}

const Name&
KoNDNStrategy::getStrategyName()
{
  static Name strategyName("/localhost/nfd/strategy/kondn/%FD%05");
  return strategyName;
}

void
KoNDNStrategy::afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
                                    const shared_ptr<pit::Entry>& pitEntry)
{
  RetxSuppressionResult suppression = m_retxSuppression.decidePerPitEntry(*pitEntry);
  if (suppression == RetxSuppressionResult::SUPPRESS) {
    NFD_LOG_DEBUG(interest << " from=" << ingress << " suppressed");
    return;
  }

  Name protocol = interest.getProtocol();
  NFD_LOG_DEBUG(interest << " protocol is " << protocol.toUri());
  NFD_LOG_DEBUG(interest << " name: " << interest.getName().toUri()
                         << " hash: " << interest.getHashedName().toUri());

  const fib::Entry& fibEntry = this->lookupFib(*pitEntry, this->getNodeID().toUri().substr(1));
  const fib::NextHopList& nexthops = fibEntry.getNextHops();
  auto it = nexthops.end();

  if (suppression == RetxSuppressionResult::NEW) {
    // forward to nexthop with lowest cost except downstream
    it = std::find_if(nexthops.begin(), nexthops.end(), [&](const auto& nexthop) {
      return isNextHopEligible(ingress.face, interest, nexthop, pitEntry);
    });

    if (it == nexthops.end()) {
      NFD_LOG_DEBUG(interest << " from=" << ingress << " noNextHop");
      if (protocol.toUri() == "/kademlia") {
        // std::cout << "********** SWITCH TO NDN ROUTING **********\n";

        const ndn::time::system_clock::TimePoint now = time::system_clock::now();
        std::cout << time::toUnixTimestamp(now) << ",AN,"
                  << "CURR_NODE: " << m_nodeId.toUri() << " INAME: " << interest.getName().toUri()
                  << std::endl;

        interest.setProtocol("ndn");
        interest.setAgentNodeID(this->getNodeID());

        std::string agentNode = interest.getAgentNodeID().toUri();

        const fib::Entry& newFibEntry = this->lookupFib(*pitEntry);
        const fib::NextHopList& ndnNexthops = newFibEntry.getNextHops();

        it = std::find_if(ndnNexthops.begin(), ndnNexthops.end(), [&](const auto& nexthop) {
          return isNextHopEligible(ingress.face, interest, nexthop, pitEntry, this->getNodeID());
        });
        auto egress = FaceEndpoint(it->getFace(), 0);
        this->sendInterest(pitEntry, egress, interest);
        return;
      }
      else if (protocol.toUri() == "/ndn") {
        lp::NackHeader nackHeader;
        nackHeader.setReason(lp::NackReason::NO_ROUTE);
        this->sendNack(pitEntry, ingress, nackHeader);

        this->rejectPendingInterest(pitEntry);
        return;
      }
    }

    auto egress = FaceEndpoint(it->getFace(), 0);
    NFD_LOG_DEBUG(interest << " from=" << ingress << " newPitEntry-to=" << egress);
    this->sendInterest(pitEntry, egress, interest);
    return;
  }

  // find an unused upstream with lowest cost except downstream
  it = std::find_if(nexthops.begin(), nexthops.end(), [&](const auto& nexthop) {
    return isNextHopEligible(ingress.face, interest, nexthop, pitEntry, true,
                             time::steady_clock::now());
  });

  if (it != nexthops.end()) {
    auto egress = FaceEndpoint(it->getFace(), 0);
    this->sendInterest(pitEntry, egress, interest);
    NFD_LOG_DEBUG(interest << " from=" << ingress << " retransmit-unused-to=" << egress);
    return;
  }

  // find an eligible upstream that is used earliest
  it = findEligibleNextHopWithEarliestOutRecord(ingress.face, interest, nexthops, pitEntry);
  if (it == nexthops.end()) {
    NFD_LOG_DEBUG(interest << " from=" << ingress << " retransmitNoNextHop");
  }
  else {
    auto egress = FaceEndpoint(it->getFace(), 0);
    this->sendInterest(pitEntry, egress, interest);
    NFD_LOG_DEBUG(interest << " from=" << ingress << " retransmit-retry-to=" << egress);
  }
}

void
KoNDNStrategy::afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
                                const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(ingress.face, nack, pitEntry);
}

} // namespace fw
} // namespace nfd
