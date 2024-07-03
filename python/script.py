import sys

import pymd as p

file = p.MappedFile("../data/01302019.NASDAQ_ITCH50")
print(f'file: {file}, size: {file.size:,} bytes')

reader = p.BinaryDataReader(file)
print(f'reader: {reader}, remaining: {reader.remaining:,} bytes')

symbols = p.Symbols(reader)
print(f'symbols: {symbols}')

print(f'reader: {reader}, remaining: {reader.remaining:,} bytes')

#lobs = p.LOBCollection()
#lob0 = lobs.getRefefence(0)
