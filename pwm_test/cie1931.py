INPUT_SIZE = 10       # Input integer size
OUTPUT_SIZE = 900      # Output integer size
INT_TYPE = 'uint16_t'
TABLE_NAME = 'cie1931';

def cie1931(L):
    L = L*100.0
    if L <= 8:
        return (L/903.3)
    else:
        return ((L+16.0)/116.0)**3

x = range(0,int(INPUT_SIZE+1))
y = [round(cie1931(float(L)/INPUT_SIZE)*OUTPUT_SIZE) for L in x]

f = open('cie1931.h', 'w')
f.write('// CIE1931 correction table\n')
f.write('// Automatically generated\n\n')

f.write('%s %s[%d] = {\n' % (INT_TYPE, TABLE_NAME, INPUT_SIZE+1))
f.write('\t')
#for i,L in enumerate(sorted(y, key=int, reverse=True)):
for i,L in enumerate(y):
    f.write('%d, ' % (900 - int(L)))
f.write('\n};\n\n')