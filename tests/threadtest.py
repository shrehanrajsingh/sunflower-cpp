import threading as th
import time


def test(a):
    print(a)


while 1:
    a0 = th.Thread(target=test, args=(1,))
    a0.start()
    a0.join()
