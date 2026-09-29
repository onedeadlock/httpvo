#!/bin/python3
"""
    AUTUOR: MICHAEL SAVIOUR
    SCRIPT: GENERATE SHUFFLE TABLE FOR VALID TCHAR CLASSIFICATION USING PSHUFB

    Each byte B is inserted into two tables (HI[] and LO[]) by representing its low and high 4 bits
    with uniques values such that HI[B >> 4] & LO[B & 0xf] is non-zero.
    This is a common technique often used in classifying batch of bytes with vector instructions
    My comments may be enough to understand the table generation process, else a quick google search should help
"""

# fill TCHAR list with expected tchar
TCHAR = [i for i in range(0, 256) if chr(i) in "!#$%&'*+-.0123456789^_`|~" or i in range(65,  91) or i in range(97, 123)]

# Split each byte into 4 bits low and hi nibble
# each high nibble correspond to the set of low nibbles that share the same high nibble and are valid tchar
ROW = {hi: set(lo for lo in range(16) if (hi << 4 | lo) in TCHAR) for hi in range(16)}

# Collect only distinct rows of low bits
DISTINCT_ROW = []
for i in range(16):
    row_i = ROW[i]
    DISTINCT_ROW.append(row_i if row_i and row_i not in DISTINCT_ROW else 0)

# Ensure that we only have atmost 8 distinct set then give each distinct row an id and build HI table from it.
# We expect only 8 distinct set, that is, we can can only correctly classify 8 distinct sets of bytes.
# SInce a byte only contains 8 bits, ID must be lesser that 8, else `1 << ID` would overflow
if len([x for x in DISTINCT_ROW if x != 0]) > 8:
    print("More than 8 distinct sets")
    exit(-1)
# give each valid high nibble a distinct id
HI_TABLE = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
for i, row in ROW.items():
    if row in DISTINCT_ROW:
        HI_TABLE[i] = 1 << i

# build LOW table by accumulating ids common to low nibble
LO_TABLE = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
for lo in range(16):
    for i, row in enumerate(DISTINCT_ROW):
        if row != 0 and lo in row:
            # include all the ids of the rows that is common to the low nibble
            LO_TABLE[lo] |= HI_TABLE[i]

# Check correctness of table over bytes in 0 - 255
for i in range(256):
    if (i in TCHAR) and (LO_TABLE[i & 0xf] & HI_TABLE[i >> 4]) == 0:
        print("Bug in classification table")
        exit(-1)

    # outside bytes must also fail
    if (i not in TCHAR) and (LO_TABLE[i & 0xf] & HI_TABLE[i >> 4]) != 0:
        print("Bug in classification table: Caught outside bytes.")
        exit(-1)

# DONE
# print in hex
print( [hex(i) for i in HI_TABLE] )
print( [hex(i) for i in LO_TABLE] )