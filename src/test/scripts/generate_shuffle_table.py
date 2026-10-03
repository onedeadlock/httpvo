#!/bin/python3
"""
    AUTUOR: MICHAEL SAVIOUR
    SCRIPT: GENERATE SHUFFLE TABLE FOR VALID/NON-VALID TCHAR CLASSIFICATION USING PSHUFB

    Each byte B is inserted into two tables (HI[] and LO[]) by representing its low and high 4 bits
    with uniques values such that HI[B >> 4] & LO[B & 0xf] is non-zero.
"""

def generate_table(CHAR_CLASS):
    # Split each byte into 4 bits low and hi nibble and create a 16x16 grid where
    # each row correspond to the set of low nibbles that shares the same top (high) nibble and are valid tchar
    HI_ROW_x_LO_COL = {hi: set(lo for lo in range(16) if (hi << 4 | lo) in CHAR_CLASS) for hi in range(16)}

    # Collect only distinct rows of low bits
    DISTINCT_ROW = []
    for i in range(16):
        row_i = HI_ROW_x_LO_COL[i]
        DISTINCT_ROW.append(row_i if row_i and row_i not in DISTINCT_ROW else 0)

    # Ensure that we only have atmost 8 distinct sets of low nibbles
    if len([x for x in DISTINCT_ROW if x != 0]) > 8:
        print("More than 8 distinct sets")
        exit(-1)

    # give each valid high nibble (row) a distinct id
    HI_TABLE = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    i = 0
    for hi, row_of_low_nib_bytes_that_has_hi in HI_ROW_x_LO_COL.items():
        if row_of_low_nib_bytes_that_has_hi in DISTINCT_ROW:
            HI_TABLE[hi] = 1 << i
            i += 1

    # build low nibble table by accumulating the ids of rows common to each low nibble
    LO_TABLE = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    for lo in range(16):
    #   include all the ids of every row has lo
        for hi, row_of_low_nib_bytes_that_has_hi in enumerate(DISTINCT_ROW):
            if row_of_low_nib_bytes_that_has_hi != 0 and lo in row_of_low_nib_bytes_that_has_hi:
                LO_TABLE[lo] |= HI_TABLE[hi]

    # Check correctness of table over bytes in 0 - 255
    for i in range(256):
        if (i in CHAR_CLASS) and (LO_TABLE[i & 0xf] & HI_TABLE[i >> 4]) == 0:
            print("Bug in classification table")
            exit(-1)

        # outside bytes must also fail
        if (i not in CHAR_CLASS) and (LO_TABLE[i & 0xf] & HI_TABLE[i >> 4]) != 0:
            print("Bug in classification table: Caught outside bytes.")
            exit(-1)
        
        return ((HI_TABLE, LO_TABLE), ([hex(i) for i in HI_TABLE], [hex(i) for i in LO_TABLE]))

# DONE
# TCHAR table for header names
TCHAR = [i for i in range(0, 256) if chr(i) in "!#$%&'*+-.0123456789^_`|~" or i in range(65,  91) or i in range(97, 123)]

print(generate_table(TCHAR))

# NON-TCHAR table for header names
NON_TCHAR_PLUS_COLON = [i for i in range(0, 256) if i not in TCHAR or chr(i) != ':']

print(generate_table(NON_TCHAR_PLUS_COLON))
