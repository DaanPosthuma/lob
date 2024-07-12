import sys

import pymd as p

file = p.MappedFile("../data/01302019.NASDAQ_ITCH50")
reader = p.BinaryDataReader(file)
symbols = p.Symbols(reader)
bmgr = p.ItchBooksManager()
strategy = p.TestStrategy(100)

idx_market_start = reader.curr()
def reset_reader():
    reader.reset(idx_market_start)

print(f'file: {file}')
print(f'reader: {reader}')
print(f'symbols: {symbols}')
print(f'bmgr: {bmgr}')
print(f'strategy: {strategy}')

p.runTestStrategy(reader, bmgr, strategy, symbols, 1000000)
reset_reader()

