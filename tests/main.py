import os
import subprocess
path = os.path


PANKTI_EXE = path.join(os.curdir , "zig-out/bin/pankti")

def get_file_path(name : str) -> str:
    return path.join(os.curdir , "tests" , "samples" , name) + ".pank"

def run_pankti(src : str) -> str:
    p = get_file_path(src)
    #print(PANKTI_EXE)
    if not path.exists(PANKTI_EXE):
        print("Pankti Exe not found!")
        raise FileNotFoundError
    if not path.exists(p):
        print("Sample File not found!")
        raise FileNotFoundError

    res = subprocess.run([PANKTI_EXE , p] , stdout=subprocess.PIPE)
    return res.stdout.decode()
