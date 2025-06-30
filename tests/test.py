class Test:
    def test_fun(self):
        print("Inside test_fun")
    
    def __del__(self):
        print("Destructor")

q = None
def t():
    global q
    o = Test()
    q = o.test_fun

t()
print(q())