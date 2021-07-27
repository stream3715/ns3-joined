/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2018 Regents of the University of California.
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

#ifndef NDN_SEND_INFO_HPP
#define NDN_SEND_INFO_HPP

#include "ndn-cxx/name-component.hpp"
#include "ndn-cxx/encoding/block.hpp"
#include "ndn-cxx/encoding/encoding-buffer.hpp"
#include "ndn-cxx/name.hpp"

#include <list>

namespace ndn {

class SendInfo
{
public:
  class Error : public tlv::Error
  {
  public:
    using tlv::Error::Error;
  };

  SendInfo();

  /**
   * @brief Create from wire encoding
   */
  explicit
  SendInfo(const Block& block);

  template<encoding::Tag TAG>
  size_t
  wireEncode(EncodingImpl<TAG>& encoder) const;

  const Block&
  wireEncode() const;

  void
  wireDecode(const Block& wire);

public: // getter/setter
  /** @brief return ContentType
   *
   *  If ContentType element is omitted, returns \c tlv::ContentType_Blob.
   */
  uint32_t
  getType() const
  {
    return m_type;
  }

  /** @brief set ContentType
   *  @param type a number defined in \c tlv::ContentTypeValue
   */
  SendInfo&
  setType(uint32_t type);

  /** @brief get send Destination
   */
  Name
  getDestination() const
  {
      return m_dest;
  }

  /** @brief set send Destination
   *  @param type a Name
   */
  SendInfo&
  setDestination(Name name);

private:
  uint32_t m_type;
  mutable Name m_dest;

  mutable Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(SendInfo);

std::ostream&
operator<<(std::ostream& os, const SendInfo& info);

} // namespace ndn

#endif // NDN_SEND_INFO_HPP
