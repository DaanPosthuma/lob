#pragma once

#include "Simulator.h"

namespace md {
class BinaryDataReader;
}

namespace simulator {

Simulator::EventT getNextMarketDataEvent(md::BinaryDataReader& reader, auto& bmgr);
void runTest(md::BinaryDataReader& reader, int numIters, bool singleThreaded);

}  // namespace simulator
