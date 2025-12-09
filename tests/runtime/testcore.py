import os
import subprocess
import tempfile
import unittest

SamplesFolder = "samples"


class PanktiTestCase(unittest.TestCase):
    def __init__(self, methodName: str = "runTest") -> None:
        super().__init__(methodName)

    def assert_goldens(self, output: tuple[str, str]) -> None:
        output_lines, golden_lines = get_golden_lines(output)
        # self.assertEqual(len(output_lines), len(golden_lines))
        for i, (out, gold) in enumerate(zip(output_lines, golden_lines)):
            self.assertEqual(out.strip(), gold.strip(), f"line number -> {i}")

    def run_file(
        self, script_path: str, pankti_bin: str | None = None, timeout: int = 10
    ) -> str:
        return runner_file(script_path, pankti_bin, timeout)

    def run_raw(
        self, script: str, pankti_bin: str | None = None, timeout: int = 10
    ) -> str:
        return runner_raw(script, pankti_bin, timeout)

    def run_golden(
        self, name: str, pankti_bin: str | None = None, timeout: int = 10
    ) -> tuple[str, str]:
        return runner_golden(name, pankti_bin, timeout)

    def get_golden_lines(
        self, t: tuple[str, str]
    ) -> tuple[list[str], list[str]]:
        return get_golden_lines(t)

    def golden(
        self, name: str, pankti_bin: str | None = None, timeout: int = 10
    ) -> None:
        output = self.run_golden(name, pankti_bin, timeout)
        self.assert_goldens(output)


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


def get_golden_lines(t: tuple[str, str]) -> tuple[list[str], list[str]]:
    output_lines = t[0].splitlines()
    golden_lines = t[1].splitlines()

    return (output_lines, golden_lines)
