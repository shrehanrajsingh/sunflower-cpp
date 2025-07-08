write = print
SHOPPING_CART = 0

APPLE =     1 << 0
BANANA =    1 << 1
CABBAGE =   1 << 2
ORANGE =    1 << 3
JUICE =     1 << 4

SHOPPING_CART = SHOPPING_CART | APPLE
SHOPPING_CART = SHOPPING_CART | BANANA

if SHOPPING_CART & APPLE:
    write ("Apple is in cart")

if SHOPPING_CART & BANANA:
    write ("Banana is in cart")

if not SHOPPING_CART & JUICE:
    write ("Juice is not in cart")

SHOPPING_CART = SHOPPING_CART | JUICE

if SHOPPING_CART & JUICE:
    write ("Juice is in cart")

SHOPPING_CART = SHOPPING_CART & ~JUICE

if SHOPPING_CART & JUICE:
    write ("Juice is in cart")
else:
    write ("Juice is not in cart")