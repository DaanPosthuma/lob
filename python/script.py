import sys
import time

import pymd as p

file = p.MappedFile("../data/01302019.NASDAQ_ITCH50")
reader = p.BinaryDataReader(file)
symbols = p.Symbols(reader)
strategy = p.TestStrategy(100)

idx_market_start = reader.curr()
def reset_reader():
    reader.reset(idx_market_start)

print(f'file: {file}')
print(f'reader: {reader}')
print(f'symbols: {symbols}')
print(f'strategy: {strategy}')

ts = time.time()
p.testStrategy(reader, strategy, symbols.byName('QQQ'), 10000000) # 4294967295
te = time.time()
print(f'took {te-ts} s' )

reset_reader()
