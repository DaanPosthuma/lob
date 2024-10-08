import sys
import os
import time
import matplotlib.pyplot as plt
import datetime
import time

import pymd as p

print(f'pid: {os.getpid()}')
#time.sleep(10)

logger = None # p.Logger(1024, datetime.timedelta(milliseconds=1))

file = p.MappedFile("../data/01302019.NASDAQ_ITCH50")
reader = p.BinaryDataReader(file)
symbols = p.Symbols(reader)
oms = p.OMS()
symbol_id = symbols.byName('QQQ')
ks = [10, 50, 100, 200, 500]
strategies = {symbol_id: [p.TestStrategy(oms, k, symbol_id, logger) for k in ks]}

print(f'file: {file}')
print(f'reader: {reader}')
print(f'symbols: {symbols}')
print(f'strategies: {strategies}')

N = 1000000

ts = time.time()
p.testStrategies(reader, strategies, N)
te = time.time()

print(f'Took {te - ts}s' )

for id, strategiesForSymbol in strategies.items():
    print(f'strategies for symbol {id} ({symbols.byId(id)})')
    for strategy in strategiesForSymbol:
        print(strategy.diagnostics.toString())

timestamps, bids, asks = p.getTopOfBookData(reader, symbol_id, N)

def plotBA(n=0):
    timestamps_ = [t.total_seconds() for t in timestamps]
    plt.plot(timestamps_[n:], bids[n:])
    plt.plot(timestamps_[n:], asks[n:])
    plt.show()

#plotBA()
