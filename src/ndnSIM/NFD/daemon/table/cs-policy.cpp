/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cs-policy.hpp"
#include "cs.hpp"
#include "common/logger.hpp"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

NFD_LOG_INIT(CsPolicy);

namespace nfd {
namespace cs {

Policy::Registry&
Policy::getRegistry()
{
  static Registry registry;
  return registry;
}

unique_ptr<Policy>
Policy::create(const std::string& policyName)
{
  Registry& registry = getRegistry();
  auto i = registry.find(policyName);
  return i == registry.end() ? nullptr : i->second();
}

std::set<std::string>
Policy::getPolicyNames()
{
  std::set<std::string> policyNames;
  boost::copy(getRegistry() | boost::adaptors::map_keys,
              std::inserter(policyNames, policyNames.end()));
  return policyNames;
}

Policy::Policy(const std::string& policyName)
  : m_policyName(policyName)
{
}

void
Policy::setLimit(size_t nMaxEntries)
{
  NFD_LOG_INFO("setLimit " << nMaxEntries);
  m_limit = nMaxEntries;
  this->evictEntries();
}

void
Policy::afterInsert(EntryRef i, bool isAgent)
{
  BOOST_ASSERT(m_cs != nullptr);
  this->doAfterInsert(i, isAgent);
}

void
Policy::afterRefresh(EntryRef i, bool isAgent)
{
  BOOST_ASSERT(m_cs != nullptr);
  this->doAfterRefresh(i, isAgent);
}

void
Policy::beforeErase(EntryRef i, bool isAgent)
{
  BOOST_ASSERT(m_cs != nullptr);
  this->doBeforeErase(i, isAgent);
}

void
Policy::beforeUse(EntryRef i, bool isAgent)
{
  BOOST_ASSERT(m_cs != nullptr);
  this->doBeforeUse(i, isAgent);
}

} // namespace cs
} // namespace nfd
