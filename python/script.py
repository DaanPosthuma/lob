import sys

import pymd

fs = pymd.FileStreamInput("aap")
buff = pymd.RingBuffer(0)
producer = buff.getProducer()
consumer = buff.createConsumer()
strategy = pymd.ConcreteStrategy("my strategy")
