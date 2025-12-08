import os
import subprocess
import tempfile

SamplesFolder = "samples"


def as_float(a: str) -> float:
    return float(a)


def runner_file(
    script_path: str, pankti_bin: str | None = None, timeout: int = 10
) -> str:
    pankti_exe: str = (
        pankti_bin
        or os.environ.get("PANKTI_BIN")
        or os.path.join("build", "pankti")
    )
    if not os.path.isabs(pankti_exe):
        pankti_exe = os.path.abspath(pankti_exe)

    if not os.path.exists(pankti_exe):
        raise Exception("Pankti Binary Not Found")

    try:
        proc = subprocess.run(
            [pankti_exe, script_path],
            capture_output=True,
            text=True,
            timeout=timeout,
        )
    except Exception as e:
        raise e

    if proc.returncode != 0:
        raise Exception(
            f"Failed to run test script file with return code {proc.returncode}"
        )

    return proc.stdout.strip()


def runner_raw(
    script: str, pankti_bin: str | None = None, timeout: int = 10
) -> str:
    pankti_exe: str = (
        pankti_bin
        or os.environ.get("PANKTI_BIN")
        or os.path.join("build", "pankti")
    )
    if not os.path.isabs(pankti_exe):
        pankti_exe = os.path.abspath(pankti_exe)

    if not os.path.exists(pankti_exe):
        raise Exception("Pankti Binary Not Found")

    with tempfile.NamedTemporaryFile(
        mode="w+", prefix="temp_", suffix=".pank", delete=True
    ) as temp:

        _ = temp.write(script)
        temp.flush()

        try:
            proc = subprocess.run(
                [pankti_exe, temp.name],
                capture_output=True,
                text=True,
                timeout=timeout,
            )
        except Exception as e:
            raise e

        if proc.returncode != 0:
            raise Exception(
                f"Failed to run test script file with return code {proc.returncode}"
            )

        return proc.stdout.strip()


def runner_golden(
    name: str, pankti_bin: str | None = None, timeout: int = 10
) -> tuple[str, str]:
    script_name = name + ".pank"
    golden_file_name = name + ".golden" + ".pank"

    tests_path = os.path.dirname(__file__)

    script_path = os.path.abspath(
        os.path.join(tests_path, SamplesFolder, script_name)
    )
    golden_file_path = os.path.abspath(
        os.path.join(tests_path, SamplesFolder, golden_file_name)
    )

    if not os.path.exists(script_path):
        raise Exception(f"Script file not found {script_path}")

    if not os.path.exists(golden_file_path):
        raise Exception("Golden file not found")

    script_output = runner_file(script_path, pankti_bin, timeout)

    golden_f = open(golden_file_path, "r")
    golden_output = golden_f.read(-1)
    golden_f.close()

    return (script_output, golden_output)
