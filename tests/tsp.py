def total_cost (mask, pos, n, cost):
    if mask == (1 << n) - 1:
        return cost[pos][0]
    
    ans = 1000000
    for i in range(n):
        if not (mask & (1 << i)):
            ans = min (ans, cost[pos][i] + total_cost (
                mask | (1 << i), i, n, cost
            ))
    
    return ans

def tsp (cost):
    n = len (cost)
    return total_cost (1, 0, n, cost)

cost = [
    [0, 10, 15, 20],
    [10, 0, 35, 25],
    [15, 35, 0, 30],
    [20, 25, 30, 0]
]

print (tsp (cost))