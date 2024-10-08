#pragma once

#include "Simulator.h"

namespace md {
class BinaryDataReader;
}

namespace md::utils {
class Symbols;
}

namespace logging {
class Logger;
}

namespace simulator {

class ItchBooksManager;

Simulator::EventT getNextMarketDataEvent(md::BinaryDataReader& reader, ItchBooksManager& bmgr);
void runTest(md::BinaryDataReader& reader, md::utils::Symbols const& symbols, int numIters, bool singleThreaded, logging::Logger* logger);

}  // namespace simulator
