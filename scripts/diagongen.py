#!/bin/env python3

import re
from typing import override

IME_PREFIX = "অভ্যন্তরীণ গোলমাল: "

INPUT_FILE: str = "src/tmpl/diagonmsg.txt"
HEADER_TEMPLATE_PATH: str = "src/tmpl/diagon.tmpl.h"
HEADER_OUTPUT_PATH: str = "src/gen/diagon.h"
SCRIPT_TEMPLATE_PATH: str = "src/tmpl/diagon.tmpl.c"
SCRIPT_OUTPUT_PATH: str = "src/gen/diagon.c"
DEF_SEPARATOR = "|"

C_CATEGORY_MAP = {
    "lexer": "PAN_DIAG_LEXER",
    "parser": "PAN_DIAG_PARSER",
    "compiler": "PAN_DIAG_COMPILER",
    "runime": "PAN_DIAG_RUNTIME",
    "str": "PAN_DIAG_STRESCAPE",
}

C_SEVERITY_MAP = {"err": "PAN_DIAG_SEV_ERROR", "warn": "PAN_DIAG_SEV_WARN"}

TemplateHeaderStr: str = ""
TemplateScriptStr: str = ""
DiagonList: list[DiagonInfo] = []

# https://stackoverflow.com/a/71790183
# C format specifier in string matching
FormatStrRegex = re.compile(
    r"\%[0 #+-]?[0-9*]*\.?\d*[hl]{0,2}[jztL]?[diuoxXeEfgGaAcpsSn%]"
)


class DiagonInfo:
    category: str = ""
    severity: str = ""
    code: str = ""
    msg: str = ""
    hint: str = ""
    format: bool = False

    @override
    def __str__(self) -> str:
        return f"[{self.category.upper()}_{self.code.upper()}][{self.severity}]:{self.msg}:{self.hint}:{self.format}"

    def to_array_item(self) -> str:
        c_category = C_CATEGORY_MAP[self.category]
        c_severity = C_SEVERITY_MAP[self.severity]
        c_code = f"{self.category.upper()}_{self.code.upper()}"
        c_formatted = f"{str(self.format).lower()}"
        c_msg = f'"{self.msg}"'
        c_hint = f'"{self.hint}"'
        result = "    {"
        result += f"{c_code}, {c_category}, {c_severity}, {c_formatted}, {c_msg}, {c_hint}"

        result += "},"
        return result

    def get_code_str(self) -> str:
        c_code = f"    // {self.msg}\n"
        c_code += f"    {self.category.upper()}_{self.code.upper()},"
        return c_code


def load_header_template(tmpl_path: str = HEADER_TEMPLATE_PATH):
    global TemplateHeaderStr
    with open(tmpl_path) as f:
        TemplateHeaderStr = f.read()


def load_script_template(tmpl_path: str = SCRIPT_TEMPLATE_PATH):
    global TemplateScriptStr
    with open(tmpl_path) as f:
        TemplateScriptStr = f.read()


def process_template(tmpl_path: str = INPUT_FILE):
    global DiagonList
    with open(tmpl_path, "r") as f:
        for line in f.readlines():
            items = line.split(DEF_SEPARATOR)
            if len(items) != 5:
                continue
            dg = DiagonInfo()
            dg.category = items[0]
            dg.severity = items[1]
            dg.code = items[2]
            if dg.code.find("ime_") > -1:
                dg.msg = IME_PREFIX + items[3]
            else:
                dg.msg = items[3]
            if FormatStrRegex.findall(dg.msg):
                dg.format = True
            hint = items[4]
            if hint[0] == "\n":
                dg.hint = ""
            else:
                dg.hint = hint.strip()
            DiagonList.append(dg)


def get_output(l: list[DiagonInfo] = DiagonList) -> list[str]:
    lists = ""
    headers = ""
    for item in l:
        lists += item.to_array_item() + "\n"
        headers += item.get_code_str() + "\n"

    return [headers, lists]


def main():
    load_header_template()
    load_script_template()
    process_template()
    output = get_output()
    header = output[0]
    lists = output[1]

    output_header = TemplateHeaderStr.replace("//REPLACEME", header)
    output_script = TemplateScriptStr.replace("//REPLACEME", lists)

    with open(HEADER_OUTPUT_PATH, "w") as f:
        _ = f.write(output_header)
    with open(SCRIPT_OUTPUT_PATH, "w") as f:
        _ = f.write(output_script)


main()
