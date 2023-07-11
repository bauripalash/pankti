#! /bin/env python3

import hashlib
import os
import sys 


def main():
    EXE_FILE = "zig-out/bin/pankti"
    if sys.platform.startswith("win32") or sys.platform.startswith("cygwin"):
        EXE_FILE+='.exe'

    CSUM_FILE = EXE_FILE + ".sha512"

    if len(sys.argv) == 2 and sys.argv[1] == "clean":
        if os.path.exists(CSUM_FILE):
            print("[+] Removed Checksum File " + CSUM_FILE)
            os.remove(CSUM_FILE)
        else:
            print("[-] Checksum File Does Not Exist " + CSUM_FILE)
        exit(1)
    
    with open(EXE_FILE , "rb") as f:
        bts = f.read()
        rh = hashlib.sha512(bts);
        with open(CSUM_FILE , "w") as wf:
            wf.write(rh.hexdigest())
            print("[+] Created Checksum File " + CSUM_FILE)


if __name__ == "__main__":
    main()
