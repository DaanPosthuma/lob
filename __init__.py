import sys
sys.path.append(r'./build/lob/pymd/RelWithDebInfo')

import pymd as md
#
#reader = md.ItchReader(r"./data/01302019.NASDAQ_ITCH50")
#
#count = 0
#def count_message(message):
#    global count
#    count += 1
#    #print(f'add_order')
#    #print(f'message: {message}')
#        
#reader.onMessage('A', count_message)
##reader.onMessage('F', add_order)
#
#num_read = reader.read(1000000) # 368366634
#print(f'num_read: {num_read}')
#print(f'count: {count}')

import pymdf as mdf

c1 = mdf.Condition("c1")
c2 = mdf.Condition("c2")

cand = c1 & c2
cor = c1 | c2

