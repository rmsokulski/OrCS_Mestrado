import sys
import os
import subprocess
import glob
import shutil

tracerFolder = "./extras/pinplay/sinuca_tracer"

if len(sys.argv) != 4:
    print("Correct use: python3 {} <Command to be traced> <Resultant traces name> [flags]".format(sys.argv[0]))
    print("If any .so file was not found, please compile sinuca_tracer executing a make inside extras/pinplay/sinuca_tracer folder")
    print("Available flags:")
    print("--controlled_tracing")
    print(" Disable tracing at the program start. So the tracing is disabled and enabled with the ORCS_tracing_start and ORCS_tracing_stop functions.")
    exit(1)

# Obtaining arguments
command = sys.argv[1]
output_name = sys.argv[2]

controlled_tracing = False
for flag_id in range(3, len(sys.argv)):
    if sys.argv[flag_id] == "--controlled_tracing":
        controlled_tracing = True


pin_cmd = ["../../../pin", "-t", "../bin/intel64/sinuca_tracer.so", "-trace", "x86", "-output", f"{output_name}", "-orcs_tracing", ("1" if controlled_tracing else "0")]
pin_cmd.extend(["--", command])

base_dir = os.getcwd()

# Execute pin tracer
os.chdir(tracerFolder)
subprocess.run(pin_cmd)

# Move Traces
files = glob.glob(f"{output_name}.*.gz")

for f in files:
    f_base_name = os.path.basename(f)
    shutil.move(f, f"{base_dir}/{f_base_name}")

os.chdir(base_dir)
