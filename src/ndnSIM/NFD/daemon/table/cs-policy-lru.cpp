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

#include "cs-policy-lru.hpp"
#include "cs.hpp"

namespace nfd {
namespace cs {
namespace lru {

const std::string LruPolicy::POLICY_NAME = "lru";
NFD_REGISTER_CS_POLICY(LruPolicy);

LruPolicy::LruPolicy()
  : Policy(POLICY_NAME)
{
}

LruPolicy::~LruPolicy()
{
  for (auto entryInfoMapPair : m_entryInfoMap) {
    delete entryInfoMapPair.second;
  }
}

void
LruPolicy::doAfterInsert(EntryRef i, bool isAgent)
{
  this->insertToQueue(i, true, isAgent);
  this->evictEntries();
}

void
LruPolicy::doAfterRefresh(EntryRef i, bool isAgent)
{
  this->insertToQueue(i, false, isAgent);
}

void
LruPolicy::doBeforeErase(EntryRef i, bool isAgent)
{
  m_queue.get<1>().erase(i);
  m_entryInfoMap.erase(i);
}

void
LruPolicy::doBeforeUse(EntryRef i, bool isAgent)
{
  this->insertToQueue(i, false, isAgent);
}

void
LruPolicy::evictEntries()
{
  BOOST_ASSERT(this->getCs() != nullptr);
  while (this->getCs()->size() > this->getLimit()) {
    BOOST_ASSERT(!m_queue.empty());

    // 先頭がAgent Nodeにおけるキャッシュでなくなるまでインクリメント
    for (auto it = m_queue.begin(); it != m_queue.end(); it++) {
      if (m_entryInfoMap[*it]->queueType != QUEUE_AGENT) {
        EntryRef er = *it;
        m_entryInfoMap.erase(er);
        this->emitSignal(beforeEvict, er);
        m_queue.erase(it);
        return;
      }
    }
  }
}

void
LruPolicy::insertToQueue(EntryRef i, bool isNewEntry, bool isAgent)
{
  Queue::iterator it;
  bool isNew = false;
  // push_back only if i does not exist
  std::tie(it, isNew) = m_queue.push_back(i);

  BOOST_ASSERT(isNew == isNewEntry);
  if (!isNewEntry) {
    m_queue.relocate(m_queue.end(), it);
    if (isAgent) {
      m_entryInfoMap[i]->queueType = QUEUE_AGENT;
    }
  }
  else {
    EntryInfo* entryInfo = new EntryInfo();
    entryInfo->queueType = !isAgent ? QUEUE_NORMAL : QUEUE_AGENT;
    entryInfo->queueIt = it;
    m_entryInfoMap[i] = entryInfo;
  }
}

} // namespace lru
} // namespace cs
} // namespace nfd
