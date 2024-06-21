**Trading Simulator**

**md:**
(ITCH) Market data parser (binary files). Todo: add different sources of market data (in particular live data would be good, for instance ccapi to get FIX data from crypto exchange). NASDAQ ITCH data can be downloaded from https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/.

**lob:**
Limit order book. Also contains (naive) ring buffer (based on LMAX Disruptor). Todo: check out https://github.com/lewissbaker/disruptorplus.

**strategies:**
Currently only one algorithm that doesn't actually do anything, just reads from the ring buffer. Todo: create some basic algorithms that actually place orders

**simulator:**
Pulls events from the market data. Also handles strategy simualtion events (place/cancel order). An event is a timestamp + function pair and it calls whichever function has to be executed next (either the market data or the simulation event). Todo: add OMS simulator that adds simulation events (with appropriate delay) when strategy places an order.

**pymd:**
Nothing to see here. Was meant to be python interface for market data, but is basically some early experiments with pybind11.

**tests:**
Market data and LOB/ring buffer tests. TODO: move these into md/lob subfolder and add tests for simulator/strategies.

**app:**
Executable that invokes all the code. Currently most of the code actually sits in the simulator library (function simulator::f) but this is temporary, I spent some time on refactoring today and haven't finshed this yet. TODO: add Python bindings to be able to dynamically create one or a bunch of strategies, load a market data file (or different source) and execute a strategy.

**Credits:**
ITCH parser is based on Charles Cooper's (https://github.com/charles-cooper/itch-order-book). Conan setup based on Antony Peacock's (https://github.com/Twon/Morpheus).

**TODO**
add logger
add python bindings
actually implement a simple strategy
track strategy orders in lob and generate pnl when they are executed.
profile and optimize
