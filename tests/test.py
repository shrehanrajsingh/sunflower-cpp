import threading


def test():
    a = 0


while True:
    t = threading.Thread(target=test)
    t.start()
    t.join()
