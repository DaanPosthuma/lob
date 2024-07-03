import sys

import pymd as p

file = p.MappedFile("../data/01302019.NASDAQ_ITCH50")
print(file)

reader = p.BinaryDataReader(file)
print(reader)

symbols = p.Symbols(reader)
print(symbols)

lobs = p.LOBCollection()
print(lobs)
