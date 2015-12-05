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

#include <vector>

#include "math.h"

#include "taisp-strategy.hpp"
#include "core/logger.hpp"

//NFD_LOG_INIT("TaispStrategy");

namespace nfd {
namespace fw {

const Name
  TaispStrategy::STRATEGY_NAME("ndn:/localhost/nfd/strategy/taisp");

  TaispStrategy::TaispStrategy(Forwarder& forwarder, const Name& name)
    : Strategy(forwarder, name)
{
  alpha = DEFAULT_ALPHA;
  w = DEFAULT_W;
  k = DEFAULT_K;
  b = 1;
  r.clear();
  pr.clear();
  s.clear();
  counter.clear();
  qwant.clear();
  ages.clear();
  n.clear();
  iteration = 1;
  T = 1;
}

TaispStrategy::~TaispStrategy()
{
}

static bool
canForwardToNextHop(shared_ptr<pit::Entry> pitEntry,
                    const fib::NextHop& nexthop)
{
  return pitEntry->canForwardTo(*nexthop.getFace());
}

static bool
hasFaceForForwarding(const fib::NextHopList& nexthops,
                     shared_ptr<pit::Entry>& pitEntry)
{
  return std::find_if(nexthops.begin(), nexthops.end(),
                      bind(&canForwardToNextHop, pitEntry, _1))
    != nexthops.end();
}

/*
static bool checklimiti(vector<unsigned int>& v, int idx)
{
  if ( idx >= 0 && idx < v.size() ) return true;
  else return false;
}

static bool checklimitf(vector<float>& v, int idx)
{
  if ( idx >= 0 && idx < v.size() ) return true;
  else return false;
}
*/

#define checklimiti(V, nn) ( (0 <= nn) && (nn < V.size()) )
#define checklimitf(V, nn) ( (0 <= nn) && (nn < V.size()) )

void
TaispStrategy::afterReceiveInterest(const Face& inFace,
                                    const Interest& interest,
                                    shared_ptr<fib::Entry> fibEntry,
                                    shared_ptr<pit::Entry> pitEntry)
{
//  NFD_LOG_TRACE("afterReceiveInterest");

  if (pitEntry->hasUnexpiredOutRecords()) {
    // not a new Interest, don't forward
    return;
  }

  const fib::NextHopList& nexthops = fibEntry->getNextHops();

  // Ensure there is at least 1 Face is available for forwarding
  if (!hasFaceForForwarding(nexthops, pitEntry)) {
    this->rejectPendingInterest(pitEntry);
    return;
  }

  fib::NextHopList::const_iterator selected;
  int nh = nexthops.size();
  uint64_t selectedIndex = -1;
  std::vector<float> tempr;
  tempr.clear();
  int ntotal = 4;  // total number of flows
  do {
    uint64_t currentIndex = 0;
    int counterz = 0;

    for (selected = nexthops.begin();
         selected != nexthops.end() && currentIndex != selectedIndex;
         ++selected, ++currentIndex) {
      if ( iteration == 1 )
        {
          for(int j=0;j<ntotal;j++)
            {
              r.push_back(4096.0);
              pr.push_back(0.0);
              s.push_back(1.0);
              counter.push_back(1);
              qwant.push_back(8);
              ages.push_back(1);
              n.push_back(1);
            }
        }
      else
        {
	  int nlocal = 1;
          if ( checklimiti(n, counterz) == true )
            nlocal = n[counterz];
	  if ( nlocal <= 1 ) nlocal = 1;
          if ( checklimitf(tempr, counterz) == true &&
               checklimitf(pr, counterz) == true )
            tempr[counterz] = alpha*4096 + (1.0f - alpha)*pr[counterz];
          if ( checklimitf(s, counterz) == true &&
               checklimitf(r, counterz) == true &&
               checklimiti(qwant, counterz) == true )
            s[counterz] = r[counterz]/(float)nlocal +
              w*(10 - qwant[counterz])/(float)nlocal;
          if ( s[counterz] < 0 )
            s[counterz] = 0.0f;
          if ( checklimiti(ages, counterz) == true ) {
            ages[counterz]++;
            if ( ages[counterz] > k )
            {
              ages[counterz] = 0;
              if ( checklimiti(counter, counterz) == true )
                counter[counterz] = 0;
            }
            else {
              if ( checklimiti(counter, counterz) == true )
                counter[counterz] += b;
	    }
          }
        }
    }
    float limt = -1.0;
    for(int k=0;k<(int)(s.size());k++)
      {
        if ( s[k] > limt )
          {
            limt = s[k];
            selectedIndex = k;
          }
      }
    b = (unsigned int)(floorf(limt));
    if ( b < 1 )
      b = 1;
    if ( (int)selectedIndex > nh )
      selectedIndex = 0;
// find the correct choice based on the index
    for (selected = nexthops.begin();
         selected != nexthops.end() && currentIndex != selectedIndex;
         ++selected, ++currentIndex) {
    //selected = nexthops[selectedIndex];
    }
    iteration++;
    pr = r;
    r = tempr;
  } while (!canForwardToNextHop(pitEntry, *selected));

  this->sendInterest(pitEntry, selected->getFace());
}

} // namespace fw
} // namespace nfd
