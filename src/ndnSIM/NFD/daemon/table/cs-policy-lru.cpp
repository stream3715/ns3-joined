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
  m_queue[isAgent ? QUEUE_AGENT : QUEUE_NORMAL].get<1>().erase(i);
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
    BOOST_ASSERT(!m_queue[QUEUE_NORMAL].empty());
    EntryRef i;

    if (!m_queue[QUEUE_NORMAL].empty()) {
      i = m_queue[QUEUE_NORMAL].front();
      m_queue[QUEUE_NORMAL].pop_front();
    }
    else {
      i = m_queue[QUEUE_AGENT].front();
      m_queue[QUEUE_AGENT].pop_front();
    }

    this->emitSignal(beforeEvict, i);
  }
}

/*
一般に通常領域(m_queue[QUEUE_NORMAL])のほうがエントリが多く、QUEUE_NORMALから削除で固定したため
この関数は一旦未使用
*/
void
LruPolicy::evictEntries(bool isAgent)
{
  BOOST_ASSERT(this->getCs() != nullptr);
  while (this->getCs()->size() > this->getLimit()) {
    BOOST_ASSERT(!m_queue[isAgent ? QUEUE_AGENT : QUEUE_NORMAL].empty());
    EntryRef i = m_queue[isAgent ? QUEUE_AGENT : QUEUE_NORMAL].front();
    m_queue[isAgent ? QUEUE_AGENT : QUEUE_NORMAL].pop_front();
    this->emitSignal(beforeEvict, i);
  }
}

void
LruPolicy::insertToQueue(EntryRef i, bool isNewEntry, bool isAgent)
{
  Queue::iterator it;
  bool isNew = false;
  string dataName = i->getName().toUri();
  // push_back only if i does not exist
  std::tie(it, isNew) = m_queue[isAgent ? QUEUE_AGENT : QUEUE_NORMAL].push_back(i);

  //　これは実装しておいてなんですがだいぶ謎(ASSERT試験では削除しました)
  if (isNew != isNewEntry && isAgent == false) {
    std::tie(it, isNew) = m_queue[QUEUE_AGENT].push_back(i);
  }
  // ここまで謎

  BOOST_ASSERT(isNew == isNewEntry);
  if (!isNewEntry) {
    m_queue[isAgent ? QUEUE_AGENT : QUEUE_NORMAL]
      .relocate(m_queue[isAgent ? QUEUE_AGENT : QUEUE_NORMAL].end(), it);
  }
}

} // namespace lru
} // namespace cs
} // namespace nfd
