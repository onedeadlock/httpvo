TCHAR = [i for i in range(0, 256) if chr(i) in "!#$%&'*+-.0123456789^_`|~" or i in range(65,  91) or i in range(97, 123)]

low = set(i & 0xf for i in TCHAR)
hi = set(i >> 4 for i in TCHAR)

low_bit = [i for i in range(16) if i in low] # low nibble covers 0 - 16, so we may need only one shuffle
hi_bit =  [i for i in range(16) if i in hi]

print(low)
print(hi)

table_low = [0x80 for i in range(16)]

table_hi = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
for i in range(16):
    if i in hi_bit:
        table_hi[i] = 0x80
    else:
        table_hi[i] = 0
unwanted_bytes = []
for i in range(128):
    if i in TCHAR:
        print(f'low: {hex(i & 0xf):8} {bin(i & 0xf):8} hi: {hex(i >> 4):8} {bin(i >> 4):8}')
