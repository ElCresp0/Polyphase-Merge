import matplotlib.pyplot as plt
import matplotlib.transforms as trans

def unique(list):
    ul = []
    for i in list:
        if i not in ul:
            ul.append(i)
    return ul

b = 0
re = 1
ru = 2
p = 3
rer = 4
rew = 5
br = 6
bw = 7
reo = 8
bo = 9
ep = 10
ebo = 11


labels = ["b", "number of records", "number of runs", "number of phases", \
    "number of record read operations", "number of record write operations", \
    "number of block read operations", "number of block write operations", \
    "number of record read/write operations", "number of disc operations", \
    "expected number of phases", "expected number of disc operations"]

f = open("data.csv", "r")
s = f.read()
f.close()
rows = s.split("\n")
rows.pop(0)
if len(rows[-1]) == 0:
    rows.pop(-1)

rows = [r.replace(",", ".") for r in rows]
rows = [r.split(";") for r in rows]
rows = [[float(i) for i in r] for r in rows]

bl = [r[0] for r in rows]
rel = [r[1] for r in rows]
rul = [r[2] for r in rows]
pl = [r[3] for r in rows]
rerl = [r[4] for r in rows]
rewl = [r[5] for r in rows]
brl = [r[6] for r in rows]
bwl = [r[7] for r in rows]
reol = [r[8] for r in rows]
bol = [r[9] for r in rows]
epl = [r[10] for r in rows]
ebol = [r[11] for r in rows]


rel_tmp = []
pl_tmp = []
epl_tmp = []
rerl_tmp = []
rewl_tmp = []
brl_tmp = []
bwl_tmp = []
bol_tmp = []
ebol_tmp = []

for i in range(len(bl) - 1, -1, -1):
    if bl[i] == 16.0:
        rel_tmp.append(rel[i])
        pl_tmp.append(pl[i])
        epl_tmp.append(epl[i])
        rerl_tmp.append(rerl[i])
        rewl_tmp.append(rewl[i])
        brl_tmp.append(brl[i])
        bwl_tmp.append(bwl[i])
        bol_tmp.append(bol[i])
        ebol_tmp.append(ebol[i])


plt.figure()
plt.title("Figure 1:\nactual and expected number of phases")
plt.plot(rel_tmp, pl_tmp, label = labels[p])
plt.plot(rel_tmp, epl_tmp, label = labels[ep])
plt.legend()

plt.xlabel(labels[re])
plt.ylabel(labels[p])
plt.savefig("fig1.png")
plt.show(block = False)


plt.figure()
plt.title("Figure 2:\nactual and expected number of disc operations, given b = 16")
plt.semilogy(rel_tmp, bol_tmp, label = labels[bo])
plt.semilogy(rel_tmp, ebol_tmp, label = labels[ebo])
plt.legend()

plt.xlabel(labels[re])
plt.ylabel("number of operations")
plt.savefig("fig2.png")
plt.show(block = False)


plt.figure()
plt.title("Figure 3:\nnumber of record and block operations, given b = 16")
plt.semilogy(rel_tmp, brl_tmp, label = labels[br]) # [tup[0] for tup in recs_op], [tup[1] for tup in recs_op]
plt.semilogy(rel_tmp, bwl_tmp, label = labels[bw]) # [tup[0] for tup in recs_exop], [tup[1] for tup in recs_exop]
plt.semilogy(rel_tmp, rerl_tmp, label = labels[rer])
plt.semilogy(rel_tmp, rewl_tmp, label = labels[rew])
plt.legend()

plt.xlabel(labels[re])
plt.ylabel("number of operations")
plt.savefig("fig3.png")
plt.show(block = False)


# ph,exph(b) given rec=5000; op,exop(b) given rec=5000
bl_tmp = []
bol_tmp = []
ebol_tmp = []
for i in range(len(bl) - 1, -1, -1):
    if rel[i] == 5000.0:
        bl_tmp.append(bl[i])
        bol_tmp.append(bol[i])
        ebol_tmp.append(ebol[i])

plt.figure()
plt.title("Figure 4:\nactual and expected number of disc operations, given N = 5000")
plt.semilogy(bl_tmp, bol_tmp, label = labels[bo])
plt.semilogy(bl_tmp, ebol_tmp, label = labels[ebo])
plt.legend()

plt.xlabel(labels[b])
plt.ylabel(labels[bo])
plt.savefig("fig4.png")
plt.show(block = False)

print("the program has ended, press enter")
input()