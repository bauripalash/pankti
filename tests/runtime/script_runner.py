import os
import subprocess
import tempfile

def as_float(a : str | None) -> float:
    if a is None:
        raise ValueError("Pankti's output is invalid")
    else:
        return float(a)

def runner_file(
    script_path: str, pankti_bin: str | None = None, timeout: int = 10
) -> str | None:
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
    except Exception:
        return None

    if proc.returncode != 0:
        return None

    return proc.stdout.strip()


def runner_raw(
    script: str, pankti_bin: str | None = None, timeout: int = 10
) -> str | None:
    pankti_exe: str = (
        pankti_bin
        or os.environ.get("PANKTI_BIN")
        or os.path.join("build", "pankti")
    )
    if not os.path.isabs(pankti_exe):
        pankti_exe = os.path.abspath(pankti_exe)

    if not os.path.exists(pankti_exe):
        return None

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
        except Exception:
            return None

        if proc.returncode != 0:
            return None

        return proc.stdout.strip()
