#!/bin/env python3

import re
import shutil
import subprocess
import sys

RESULT_CODE_OUTPUT = "src/gen/kwlookup.h"

# Return keyword defines from header file
def process_header(header_path: str = "") -> dict[str, str]:
    def_reg = re.compile(r"#define\s+(\w+)\s+(.*)")
    result: dict[str, str] = {}

    try:
        with open(header_path, "r") as f:
            lines = f.readlines()
            for line in lines:
                rematch = def_reg.search(line)
                if rematch:
                    name, value = rematch.groups()
                    if name.startswith("KW_"):
                        result[name] = value.strip().strip('"')

    except FileNotFoundError:
        print(f"Error : Header '{header_path}' not found!")
        return result

    return result


# Return processed gperf template file
def process_gperffile(defines: dict[str, str], gperf_path: str = "") -> str:
    result = ""
    try:
        with open(gperf_path, "r") as f:
            content = f.read()
            for item in sorted(defines.keys(), key=len, reverse=True):
                content = content.replace(item, defines[item])
            result = content
    except FileNotFoundError:
        print(f"Error : Gperf Template '{gperf_path}' not found!")
        return result

    return result

def get_gperffile() -> str:
    header_path = sys.argv[1]
    gperf_path = sys.argv[2]
    defines = process_header(header_path)
    return process_gperffile(defines, gperf_path)

def run_gperf():
    gperf_exe = shutil.which("gperf")
    if not gperf_exe:
        print("Error : gperf not found!")
        exit(1)
    gperf_input = get_gperffile()
    cmd = [
        gperf_exe,
        "-L", "ANSI-C",
        "-C",
        "-D",
        "-t",
        "-l",
        "-N", "PanktiKeywordLookup"
    ]

    try:
        proc = subprocess.run(
            cmd, 
            input=gperf_input, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE, 
            text=True,
            encoding="utf-8"
        )
    except Exception as e:
        print("Error : Failed to run gperf : ", e)
        
        raise SystemExit(1)

    if proc.returncode != 0:
        print("Error : Failed to run gperf")
        if proc.stderr:
            print("Stderr :\n", proc.stderr)
            raise SystemExit(1)

    return proc.stdout


def main():
    argc = len(sys.argv)
    if argc < 4:
        print("Usage : kwgperfgen.py <HEADER PATH> <GPERF FILE> <OUTPUT>")
        exit(1)
    result = run_gperf()
    outpath = sys.argv[3]
    with open(outpath, "w") as f:
        _ = f.write(result)

if __name__ == "__main__":
    main()
