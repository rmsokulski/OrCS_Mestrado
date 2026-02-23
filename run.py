import os
import glob
from pathlib import Path
import subprocess

TRACES_DIR = Path("../Traces")
CFG_FILE = Path("../configuration_files/skylakeServer.cfg")

RESULTS_DIR = Path("./results")
LOGS_DIR = Path("./ramulator_logs")

ORCS_BIN = "./orcs"

def main():
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    LOGS_DIR.mkdir(parents=True, exist_ok=True)

    print(f"[+] Traces dir:   {TRACES_DIR.resolve()}")
    print(f"[+] Results dir:  {RESULTS_DIR.resolve()}")
    print(f"[+] Logs dir:     {LOGS_DIR.resolve()}")

    trace_groups = collect_trace_groups(TRACES_DIR)

    print(f"[+] Found {len(trace_groups)} trace groups\n")

    for i, name in enumerate(sorted(trace_groups)):
        print(f"[{i+1}/{len(trace_groups)}] Running trace: {name}")

        trace_prefix = TRACES_DIR / name
        result_file = RESULTS_DIR / f"{name}.res"
        log_file = LOGS_DIR / f"{name}.trace.log"

        run_trace(trace_prefix, result_file, log_file)

    print("\n[✓] All simulations finished!")


def collect_trace_groups(trace_dir: Path):
    """
    Finds valid <name>.tid0.<type>.out.gz groups.
    Returns set of base trace names.
    """
    pattern = str(trace_dir / "*.tid0.*.out.gz")
    files = glob.glob(pattern)

    names = set()
    for f in files:
        base = Path(f).name
        name = base.split(".tid0.")[0]
        names.add(name)

    return names


def run_trace(trace_prefix: Path, result_file: Path, log_file: Path):
    env = os.environ.copy()

    # If ramulator output_trace.txt is created in cwd:
    tmp_log = Path("output_trace.txt")

    cmd = [
        ORCS_BIN,
        "-c", str(CFG_FILE),
        "-t", str(trace_prefix),
        "-f", str(result_file)
    ]

    print("    Command:", " ".join(cmd))

    ret = subprocess.run(cmd)

    if ret.returncode != 0:
        print(f"[!] Error running trace {trace_prefix.name}")
        return

    if tmp_log.exists():
        tmp_log.rename(log_file)
    else:
        print(f"[!] Warning: output_trace.txt not found for {trace_prefix.name}")


if __name__ == "__main__":
    main()
