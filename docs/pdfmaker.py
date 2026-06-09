#!/bin/env python
import re
import subprocess

ContentPages: list[str] = [
    "docs/_index.md",
    "docs/basics/_index.md",
    "docs/basics/variables.md",
    "docs/basics/booleans.md",
    "docs/basics/numbers.md",
    "docs/basics/string.md",
    "docs/basics/map.md",
    "docs/basics/array.md",
    "docs/basics/if_else.md",
    "docs/basics/while_loop.md",
    "docs/basics/builtins.md",
    "docs/stdlib/_index.md",
    "docs/stdlib/math.md",
    "docs/stdlib/map.md",
    "docs/stdlib/string.md",
    "docs/stdlib/array.md",
    "docs/stdlib/system.md",
    "docs/stdlib/file.md",
    "docs/stdlib/graphics.md",
]

OutputName = "output"
OutputMD = OutputName + ".md"
OutputPDF = OutputName + ".pdf"
OutputTypst = OutputName + ".typ"

BengaliNormalFont = "Noto Serif Bengali"
EnglishNormalFont = "Noto Serif"

BengaliCodeFont = "Noto Serif Bengali"
EnglishCodeFont = "monospace"

TypstCoverPage = "typstcoverpage.typ"
TypstStyleConfig = "typststyle.typ"

MarkdownRemovePatterns = ["{.dh3}", "{.args-table}", "{.return-table}"]


def strip_front_matter(content: str) -> str:
    front_pattern_toml = r"^(\+\+\+\n.*?\n\+\+\+\n)"
    front_pattern_yaml = r"^(\-\-\-\n.*?\n\-\-\-\n)"
    result = re.sub(front_pattern_toml, "", content, flags=re.DOTALL | re.MULTILINE)
    result = re.sub(front_pattern_yaml, "", result, flags=re.DOTALL | re.MULTILINE)
    # temporary solution : remote images
    result = re.sub(
        r"!\[.*?\]\(https?://.*?\)", "", result, flags=re.DOTALL | re.MULTILINE
    )
    return result.strip()


def clean_patterns(content: str) -> str:
    result = content
    for item in MarkdownRemovePatterns:
        result = result.replace(item, "")

    return result


def process_file(filepath: str) -> str:
    content = ""
    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()

    content = strip_front_matter(content)
    content = clean_patterns(content)
    content = content + "\n\n"
    return content


def merge_files(files: list[str], language: str) -> str:
    result = ""
    for file in files:
        print(f"    [*] File: {file}")
        result += process_file(f"{language}/{file}")
    return result

def build_typst_file(md : str = OutputMD, typ : str = OutputTypst) -> None:
    cmd = [
        "pandoc",
        md,
        "-o",
        typ,
        "-f",
        "markdown-yaml_metadata_block",
    ]
    try:
        _ = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(f"    [*] Typst File Generated Successfully : {typ}")
    except subprocess.CalledProcessError as e:
        print(f"    [ERR] Typst File Generation Failed : ", str(e.stderr), str(e.stdout))
    return

def process_typst_file(typ : str = OutputTypst) -> None:
    content : str = ""
    with open(TypstCoverPage, "r") as f:
        content += f.read()
    with open(TypstStyleConfig, "r") as f:
        content += f.read()
    with open(typ, "r") as f:
        content += f.read()

    with open(typ, "w") as f:
        _ = f.write(content)

def compile_typst(typ : str = OutputTypst, pdf : str = OutputPDF) -> None:
    cmd = [
        "typst",
        "compile",
        typ,
        pdf
    ]
    try:
        _ = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(f"    [*] PDF Generated Successfully : {pdf}")
    except subprocess.CalledProcessError as e:
        print(f"    [ERR] PDF Generation Failed : ", str(e.stderr), str(e.stdout))

def main() -> None:
    print(f"[+] Merging Files")
    content = merge_files(ContentPages, "bengali")
    print(f"[=] Finished Merging Files")
    with open(OutputMD, "w", encoding="utf-8") as f:
        _ = f.write(content)
    print(f"[+] Building Typst File")
    build_typst_file()
    process_typst_file()
    print(f"[+] Building PDF")
    compile_typst()
    #build_pdf()
    print(f"[=] Finished Building PDF")


if __name__ == "__main__":
    main()
