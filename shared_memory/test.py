"""
A very crude script for automatic testing of the shared memory algorithm.
"""

import subprocess
import sys
import csv

precisions = [0.01]
workers = [2]
array_sizes = [5]

attempts = 1

percentage_error_allowed = 1

results = [["Array size", "Number of workers", "Precision", "Number of tests",
"Test outcome"]]

for precision in precisions:
    for array_size in array_sizes:
        one_worker_output = subprocess.check_output(["./shared-memory.o",
                "-p",
                str(precision),
                "-w",
                "1",
                "-a",
                str(array_size)])

        one_worker_output = one_worker_output.decode("utf-8")
        one_worker_output = one_worker_output.split("Thread")[1]
        one_worker_output = one_worker_output.split("\n")

        for worker in workers:
            ok = False
            for i in range(attempts):
                p = subprocess.check_output(["./shared-memory.o",
                    "-p",
                    str(precision),
                    "-w",
                    str(worker),
                    "-a",
                    str(array_size)])
                p = p.decode("utf-8")
                p = p.split("Thread")
                for output in p:
                    this_output_is_ok = False
                    if "Set" not in output:
                        this_output_is_ok = True
                    to_process = output.split("\n")
                    for result_line, ref_line in zip(to_process, one_worker_output):
                        result_line = result_line.split(" ")
                        ref_line = ref_line.split(" ")
                        if "1.000000" in result_line:
                            for result_value, ref_value in zip(result_line, ref_line):
                                if result_value != "":
                                    # Account for floating point weirdness.
                                    if float(result_value) < (float(ref_value) - 0.001):
                                        this_output_is_ok = False

                                    # Compare the difference between reference and result value
                                    if (abs(float(result_value) - float(ref_value))) > precision:
                                        this_output_is_ok = False
                    if this_output_is_ok:
                        ok = True
                        # break

            if ok is False:
                results += [[array_size, worker, precision, attempts, "ERROR"]]
            else:
                results += [[array_size, worker, precision, attempts, "OK"]]

with open("test_output.csv", "w") as file:
    writer = csv.writer(file)
    for row in results:
        writer.writerow(row)

print(results)
