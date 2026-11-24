import sys
import os
import subprocess
import glob
import shutil

tracerFolder = "./extras/pinplay/sinuca_tracer"

if len(sys.argv) < 3:
    print("Correct use: python3 {} <Command to be traced> <Resultant traces name> [flags]".format(sys.argv[0]))
    print("If any .so file was not found, please compile sinuca_tracer executing a make inside extras/pinplay/sinuca_tracer folder")
    print("Available flags:")
    print("--controlled_tracing")
    print(" Disable tracing at the program start. So the tracing is disabled and enabled with the ORCS_tracing_start and ORCS_tracing_stop functions.")
    print("--threads N")
    print(" Number of threads to trace (one dyn/mem trace file per thread). Also sets OMP_NUM_THREADS. Default: 1.")
    exit(1)

# Obtaining arguments
command = sys.argv[1]
output_name = sys.argv[2]

controlled_tracing = False
threads = 1
flag_id = 3
while flag_id < len(sys.argv):
    if sys.argv[flag_id] == "--controlled_tracing":
        controlled_tracing = True
    elif sys.argv[flag_id] == "--threads" and flag_id + 1 < len(sys.argv):
        flag_id += 1
        threads = int(sys.argv[flag_id])
    else:
        print("Unknown flag: {}".format(sys.argv[flag_id]))
        exit(1)
    flag_id += 1


pin_cmd = ["../../../pin", "-t", "../bin/intel64/sinuca_tracer.so", "-trace", "x86", "-output", f"{output_name}", "-threads", str(threads)]
if controlled_tracing:
    # 1 = trace only between ORCS_tracing_start/stop; omitted (-1) = whole program
    pin_cmd.extend(["-orcs_tracing", "1"])
pin_cmd.extend(["--", command])

base_dir = os.getcwd()

# WORKAROUND: Pin 3.11 crashes on multi-threaded programs with glibc >= 2.34
# (clone3). noclone3 disables clone3 so glibc falls back to clone.
# Remove once Pin is upgraded.
noclone3 = os.path.join(os.path.dirname(os.path.abspath(__file__)), "noclone3")
noclone3_src = noclone3 + ".c"
if not os.path.exists(noclone3) or os.path.getmtime(noclone3) < os.path.getmtime(noclone3_src):
    subprocess.run(["cc", "-O2", noclone3_src, "-o", noclone3], check=True)
pin_cmd.insert(0, noclone3)

# Execute pin tracer
os.chdir(tracerFolder)
env = dict(os.environ, OMP_NUM_THREADS=str(threads))
subprocess.run(pin_cmd, env=env)

# Move Traces
files = glob.glob(f"{output_name}.*.gz")

for f in files:
    f_base_name = os.path.basename(f)
    shutil.move(f, f"{base_dir}/{f_base_name}")

os.chdir(base_dir)
