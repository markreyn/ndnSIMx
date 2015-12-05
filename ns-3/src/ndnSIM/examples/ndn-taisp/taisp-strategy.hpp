/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California,
 *                      Arizona Board of Regents,
 *                      Colorado State University,
 *                      University Pierre & Marie Curie, Sorbonne University,
 *                      Washington University in St. Louis,
 *                      Beijing Institute of Technology,
 *                      The University of Memphis
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
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TAISP_STRATEGY_HPP
#define TAISP_STRATEGY_HPP

#include "face/face.hpp"
#include "fw/strategy.hpp"

namespace nfd {
namespace fw {

class TaispStrategy : public Strategy {
public:
  TaispStrategy(Forwarder& forwarder, const Name& name = STRATEGY_NAME);

  virtual ~TaispStrategy();

  virtual void
  afterReceiveInterest(const Face& inFace, const Interest& interest,
                       shared_ptr<fib::Entry> fibEntry,
                       shared_ptr<pit::Entry> pitEntry);

public:
  static const Name STRATEGY_NAME;

protected:
  // algorithm parameters (set by user)
  float alpha;                  // decay rate for step 1
  float w;                      // weight used to determine buffer occupancy
  unsigned int k;               // aging rate for step 2
  // algorithm parameters (computed)
  unsigned int b;               // max value of token counters
  std::vector<float> r;         // current value of r
  std::vector<float> pr;        // previous value of r
  std::vector<float> s;         // shaping rate
  std::vector<unsigned int> counter;   // token counters
  std::vector<unsigned int> qwant;     // desired value of q size
  std::vector<unsigned int> ages;      // flow ages
  std::vector<unsigned int> n;         // value of n for this flow
  unsigned int iteration;              // which iteration?
  unsigned int T;                      // sample interval
};

#define DEFAULT_ALPHA   0.5
#define DEFAULT_W       0.8
#define DEFAULT_K         2

} // namespace fw
} // namespace nfd

#endif // TAISP_STRATEGY_HPP
