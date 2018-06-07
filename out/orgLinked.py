import sys
file = sys.argv[1]

ops = []
isns = []


with open(file) as f:
    line = ''
    for i in xrange(10):
        line = f.readline()[:-2]
        ops.append(int(line))
        line = f.readline()[:-2]
        line = line.replace(",", ".")
        isns.append(float(line))
        f.readline()

ops.sort()
isns.sort()

del ops[0]
del ops[-1]

del isns[0]
del isns[-1]

print sum(ops) / float(len(ops))
print sum(isns) / float(len(isns))
