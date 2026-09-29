#!/bin/python3

"""
    AUTUOR: MICHAEL SAVIOUR
    SCRIPT: GENERATE VALID TCHAR TABLE

    valid token character is set to 1, else 0
"""

# tchar
TCHAR = [i for i in range(0, 256) if chr(i) in "!#$%&'*+-.0123456789^_`|~" or i in range(65,  91) or i in range(97, 123)]

TABLE = [0] * 256
for i in range(256):
    if i in TCHAR:
        TABLE[i] = 1

print(TABLE)