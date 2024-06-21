#pragma once

namespace md {
class BinaryDataReader;
}

namespace simulator {
void f(md::BinaryDataReader& reader, int numIters, bool singleThreaded);
}
