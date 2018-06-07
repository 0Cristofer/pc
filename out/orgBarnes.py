import sys
file = sys.argv[1]

ctx = []
pagef = []
isns = []
branches = []
l1dm = []
llcm = []
secs = []

with open(file) as f:
    line = ''
    for i in xrange(10):
        line = f.readline()[:-2]
        line = line.replace(".", "")
        ctx.append(int(line))

        line = f.readline()[:-2]
        line = line.replace(".", "")
        pagef.append(int(line))

        line = f.readline()[:-2]
        line = line.replace(",", ".")
        isns.append(float(line))

        line = f.readline()[:-3]
        line = line.replace(",", ".")
        branches.append(float(line))

        line = f.readline()[:-3]
        line = line.replace(",", ".")
        lldm.append(float(line))

        line = f.readline()[:-3]
        line = line.replace(",", ".")
        llcm.append(float(line))

        line = f.readline()[:-3]
        line = line.replace(",", ".")
        secs.append(float(line))

        f.readline()

ctx.sort()
pagef.sort()
isns.sort()
branches.sort()
l1dm.sort()
llcm.sort()
secs.sort()

del ctx[0]
del ctx[-1]

del pagef[0]
del pagef[-1]

del isns[0]
del isns[-1]

del branches[0]
del branches[-1]

del l1dm[0]
del l1dm[-1]

del llcm[0]
del llcm[-1]

del secs[0]
del secs[-1]

print sum(ctx) / float(len(ctx))
print sum(pagef) / float(len(pagef))
print sum(isns) / float(len(isns))
print sum(branches) / float(len(branches))
print sum(l1dm) / float(len(l1dm))
print sum(llcm) / float(len(llcm))
print sum(secs) / float(len(secs))
