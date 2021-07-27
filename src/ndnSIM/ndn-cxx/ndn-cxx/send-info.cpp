/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2019 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#include "ndn-cxx/send-info.hpp"
#include "ndn-cxx/encoding/block-helpers.hpp"
#include "ndn-cxx/encoding/encoding-buffer.hpp"

namespace ndn {

BOOST_CONCEPT_ASSERT((WireEncodable<SendInfo>));
BOOST_CONCEPT_ASSERT((WireEncodableWithEncodingBuffer<SendInfo>));
BOOST_CONCEPT_ASSERT((WireDecodable<SendInfo>));
static_assert(std::is_base_of<tlv::Error, SendInfo::Error>::value,
              "SendInfo::Error must inherit from tlv::Error");

SendInfo::SendInfo()
  : m_type(tlv::SendType_Normal)
{
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(SendInfo);

SendInfo::SendInfo(const Block& block)
{
  wireDecode(block);
}

SendInfo&
SendInfo::setType(uint32_t type)
{
  m_wire.reset();
  m_type = type;
  return *this;
}

SendInfo&
SendInfo::setDestination(Name dest)
{
  m_wire.reset();
  m_dest = dest;
  return *this;
}

template<encoding::Tag TAG>
size_t
SendInfo::wireEncode(EncodingImpl<TAG>& encoder) const
{
  // SendInfo ::= META-INFO-TYPE TLV-LENGTH
  //                SendType?
  //                FreshnessPeriod?
  //                FinalBlockId?
  //                AppSendInfo*

  size_t totalLength = 0;

  // SendType
  if (m_type != tlv::SendType_Normal) {
    totalLength += prependNonNegativeIntegerBlock(encoder, tlv::SendType, m_type);
  }

  // SendDestination
  if (!m_dest.empty()) {
    totalLength += getDestination().wireEncode(encoder);
  }

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(tlv::SendInfo);
  return totalLength;
}

const Block&
SendInfo::wireEncode() const
{
  if (m_wire.hasWire())
    return m_wire;

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();
  return m_wire;
}

void
SendInfo::wireDecode(const Block& wire)
{
  m_wire = wire;
  m_wire.parse();

  // SendInfo ::= META-INFO-TYPE TLV-LENGTH
  //                SendType?
  //                FreshnessPeriod?
  //                FinalBlockId?
  //                AppSendInfo*

  auto val = m_wire.elements_begin();

  // SendType
  if (val != m_wire.elements_end() && val->type() == tlv::SendType) {
    m_type = readNonNegativeIntegerAs<uint32_t>(*val);
    ++val;
  }
  else {
    m_type = tlv::SendType_Normal;
  }
}

std::ostream&
operator<<(std::ostream& os, const SendInfo& info)
{
  // SendType
  os << "SendType: " << info.getType();

  // SendDestination
  if (!info.getDestination().empty()) {
    os << ", SendDestination: " << info.getDestination();
  }

  return os;
}

} // namespace ndn
