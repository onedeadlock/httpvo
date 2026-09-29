#!/bin/python3
"""
    GENERATE SHUFFLE TABLE FOR CONTROL CHAR CLASSIFICATION USING PSHUFB (HTAB EXCLUDED AND DEL INCLUDED)

    Each byte B is inserted into two tables (HI[] and LO[]) by representing its low and high 4 bits
    with uniques values such that HI[B >> 4] > LO[B & 0xf] is non-zero.
"