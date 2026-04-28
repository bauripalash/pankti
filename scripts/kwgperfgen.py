#!/bin/env python3

import re
import shutil
import subprocess
import sys

RESULT_CODE_OUTPUT = "src/gen/kwlookup.h"
GPERF_TEMPLATE = """%{
#include <string.h>
#include "../include/token.h"
%}

struct PanktiKeyword{
	const char * name;
	PTokenType ttype;
};
%%"""

# Return keyword defines from header file
# Result =>
# "LET:<TOKEN TYPE>" {
#   "EN" : "let",
#   "BN" : "let in bengali",
#   "PN" : "dhori"
# }
def process_header(header_path: str = "") -> dict[str, dict[str, str]]:
    def_reg = re.compile(r"#define\s+(KW_(EN|BN|PN)_(\w+))\s+\"(.*)\"")
    result: dict[str, dict[str, str]] = {}

    try:
        with open(header_path, "r") as f:
            lines = f.readlines()
            for line in lines:
                rematch = def_reg.search(line)
                if rematch:
                    _,lang, name, value = rematch.groups()
                    if name not in result:
                        result[name] = {}
                    result[name][lang] = value


    except FileNotFoundError:
        print(f"Error : Header '{header_path}' not found!")
        return result

    return result

def define_gperf(defines : dict[str, dict[str,str]]) -> str:
    lines : list[str] = []
    lines.append(GPERF_TEMPLATE)
    for name, langs in defines.items():
        token = f"T_{name}"

        kwinfo = ""
        for lang in ["BN", "EN", "PN"]:
            if lang in langs:
                kw = langs[lang]
                kwinfo += f"\"'\"KW_{lang}_{name}\"'/\""
                lines.append(f"{kw}, {token}")
        # Remove this comment to generate kwinfo lines
        #print(f"#define KWINFO_{name} {kwinfo[:]}")
    lines.append("%%\n")
    return "\n".join(lines)

def get_gperffile() -> str:
    header_path = sys.argv[1]
    #gperf_path = sys.argv[2]
    defines = process_header(header_path)
    return define_gperf(defines)
    #return process_gperffile(defines, gperf_path)

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
    _ = get_gperffile()
    argc = len(sys.argv)
    if argc < 3:
        print("Usage : kwgperfgen.py <HEADER PATH> <OUTPUT>")
        exit(1)
    result = run_gperf()
    outpath = sys.argv[2]
    with open(outpath, "w") as f:
        _ = f.write(result)

if __name__ == "__main__":
    main()
