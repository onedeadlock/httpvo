#!/bin/python3
"""
    AUTUOR: MICHAEL SAVIOUR
    SCRIPT: GENERATE SHUFFLE TABLE FOR VALID/NON-VALID TCHAR CLASSIFICATION USING PSHUFB

    Each byte B is inserted into two tables (HI[] and LO[]) by representing its low and high 4 bits
    with uniques values such that HI[B >> 4] & LO[B & 0xf] is non-zero.
"""

def generate_table(CHAR_CLASS):
    MAX_DISTINCT_ROW = 8 # We choose one bit for the ID, per distinct row, so we can only have 8 of them (to fit 1 byte) 
   
    """
    First, split each byte into 4 bits low and hi nibble and create a 16x16 grid where
    each high nibble correspond to the set of low nibbles that shares it as their high nibble
    for instance (the index is used for high):
    HI   -      LO(s)
    [0]  - 0, 1, 2, 3, 4, 5     // 0x0_0, 0x0_1, 0x0_2, 0x0_3, 0x0_4, 0x0_5
    ...          ...
    ...          ...
    [15] - 0, 1, 2, 3, 4, 5     // 0xf_0, 0xf_1, 0xf_2, 0xf_3, 0xf_4, 0xf_5
    """
    HI_ROW_x_LO_COL = [0] * 16
    for hi in range(16):
        HI_ROW_x_LO_COL[hi] = [lo for lo in range(16) if (hi << 4 | lo) in CHAR_CLASS]

    # Next, build High Table by giving each row an id. Similar rows are given the same ID
    i = 0
    DUPLICATE_ROW = []
    HI_TABLE = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    for hi, row in enumerate(HI_ROW_x_LO_COL):
        if len(row):
            if row not in DUPLICATE_ROW:
                HI_TABLE[hi] = 1 << i
                DUPLICATE_ROW.append(row)
                i += 1
            else:
                HI_TABLE[hi] = HI_TABLE[HI_ROW_x_LO_COL.index(row)]

    # Check if we have more than 8 unique rows
    if len(DUPLICATE_ROW) > MAX_DISTINCT_ROW:
        print("error, more than 8 distinct rows")
    
    # build Low Table by accumulating the id of each row common to a low nibble
    LO_TABLE = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    for lo in range(16):
    #   include all the ids of every row that has this low nibble
        for hi, row in enumerate(HI_ROW_x_LO_COL):
            if len(row) and lo in row:
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
        
    return (HI_TABLE, LO_TABLE)


def main():
    # TCHAR table for header names
    TCHAR = [i for i in range(0, 256) if chr(i) in "!#$%&'*+-.0123456789^_`|~" or i in range(65,  91) or i in range(97, 123)]
    HI, LO = generate_table(TCHAR)
    print(HI, LO)

    # NON-TCHAR table for header names
    NON_TCHAR_PLUS_COLON = [i for i in range(0, 256) if (i not in TCHAR)]
    HI, LO = generate_table(NON_TCHAR_PLUS_COLON)
    print(HI, LO)

main()