import re
import subprocess

ContentPages : list[str] = [
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
    "docs/stdlib/_index.md",
    "docs/stdlib/math.md",
    "docs/stdlib/os.md",
    "docs/stdlib/map.md",
    "docs/stdlib/string.md",
    "docs/stdlib/big.md",
]

OutputName = "output"
OutputMD = OutputName + ".md"
OutputPDF = OutputName + ".pdf"

BengaliNormalFont = "Noto Serif Bengali"
EnglishNormalFont = "Noto Serif"

BengaliCodeFont = "Noto Serif Bengali"
EnglishCodeFont = "monospace"

PandocPDFEngine = "typst"
PDFEngineTemplate = "article.typ"

def strip_front_matter(content : str) -> str:
    front_pattern_toml = r"^(\+\+\+\n.*?\n\+\+\+\n)"
    front_pattern_yaml = r"^(\-\-\-\n.*?\n\-\-\-\n)"
    result = re.sub(front_pattern_toml, '', content, flags=re.DOTALL | re.MULTILINE)
    result = re.sub(front_pattern_yaml, '', result, flags=re.DOTALL | re.MULTILINE)
    return result.strip()

def process_file(filepath: str) -> str:
    content = ""
    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()

    content = strip_front_matter(content)

    content = content + "\n\n---\n\n"
    return content

def merge_files(files : list[str], language : str) -> str:
    result = ""
    for file in files:
        print(f"    [*] File: {file}")
        result += process_file(f"{language}/{file}")
    return result

def build_pdf(md : str = OutputMD, pdf : str = OutputPDF, lang : str = "bn") -> None:
    main_font = BengaliNormalFont
    code_font = BengaliCodeFont
    if lang == "en":
        main_font = EnglishNormalFont
        code_font = EnglishCodeFont

    cmd = [
        "pandoc",
        md,
        "-o",
        pdf,
        f"--pdf-engine={PandocPDFEngine}",
        "--toc",
        "--toc-depth=3",
        "--number-sections",
        #"-V", f"template={PDFEngineTemplate}",
        "-V", f"mainfont={main_font}",
        "-V", f"monofont={code_font}",

    ]
    try:
        _ = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(f"    [*] PDF Generated Successfully : {pdf}")
    except subprocess.CalledProcessError as e:
        print(f"    [ERR] PDF Generation Failed : ", str(e.stderr), str(e.stdout))
    return

def main() -> None:
    print(f"[+] Merging Files")
    content = merge_files(ContentPages, "bengali")
    print(f"[=] Finished Merging Files")
    with open(OutputMD, "w", encoding="utf-8") as f:
        _ = f.write(content)
    print(f"[+] Building PDF")
    build_pdf()
    print(f"[=] Finished Building PDF")

if __name__ == "__main__":
    main()
