#!/usr/bin/python3
import os
import sys

current_file = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file)
sys.path.append(os.path.join(current_dir, "../../lib"))
import pyterminus

# print(pyterminus.YPCBytes.attributes)
b = pyterminus.YPCBytes(b'bce')
print(len(b))
print(type(b))

print("b is {}".format(b))
print("b is {}".format(str(b)))
c = pyterminus.CryptoPack(pyterminus.CryptoPackType.IntelSGXAndEthCompatible)
d = c.gen_ecc_private_key()
print("c is: {}".format(c))
print(dir(d))
print(str(d))
