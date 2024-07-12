#! /bin/env python3

from typing import List
import hashlib
import os
import subprocess
from zipfile import ZipFile
import zipfile

DIST_PATH = "dist"

def gen_sum(filepath : str) -> None:
    exe_file = os.path.join(os.curdir , DIST_PATH, filepath)
    csum_file = exe_file + ".sha512"
    with open(exe_file , "rb") as f:
        bts = f.read()
        rh = hashlib.sha512(bts);
        with open(csum_file , "w") as wf:
            wf.write(f"{rh.hexdigest()}  {filepath}")
            print("[*] Created Checksum File " + csum_file)

def build_release() -> bool:
    result =  subprocess.run(["make", "release"], 
                             stdout=subprocess.PIPE, 
                             stderr=subprocess.PIPE)

    return result.returncode == 0

def create_zip(filenames : List[str], output : str) -> bool:
    
    with ZipFile(output , mode="w", compression=zipfile.ZIP_DEFLATED) as z:
        for item in filenames:
            z.write(os.path.join(DIST_PATH, item), compresslevel=9)

    return True

def main():
    release_files = ["pankti-linux64", 
                     "pankti-linux32", 
                     "pankti-win32.exe", 
                     "pankti-win64.exe"]
    print("[+] Building Release")

    ok = build_release()

    if not ok:
        print("[X] Failed to build release!")
        exit(1)


    print("[+] Finished Building Release")


    for item in release_files:
        gen_sum(item)

    print("[+] Creating Zip Files")
    linux_zip = os.path.join(DIST_PATH, "pankti-linux.zip")
    print(f"[*] Zipping Linux Exes : {linux_zip}")
    create_zip(release_files[0:2], linux_zip)

    win_zip = os.path.join(DIST_PATH, "pankti-win.zip")
    print(f"[*] Zipping Windows Exes : {win_zip}")
    create_zip(release_files[2:4], win_zip)

    print("[+] Finished Creating Zip Files")


if __name__ == "__main__":
    main()
