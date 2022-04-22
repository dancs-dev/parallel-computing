"""
A basic script for automatic testing of the distributed memory algorithm.

Results will be saved to 'test_output.csv'. Output is appended, rather than
overwriting current contents.

This script must be run from the same folder as a correctly compiled sequential
and parallel version.

If any element is different, the output will be declared as an error.

Run using 'python test.py' or 'python3 test.py'.
"""

import subprocess
import csv

precisions = [0.01, 0.001]
workers = [4, 6, 10]
array_sizes = [5000, 10000]

attempts = 25

results = [["Array size", "Number of workers", "Precision", "Number of tests",
"Test outcome"]]

for precision in precisions:
    for array_size in array_sizes:
        one_worker_output = subprocess.check_output(["./sequential.o",
                "-p",
                str(precision),
                "-a",
                str(array_size)])

        one_worker_output = one_worker_output.decode("utf-8")
        one_worker_output = one_worker_output.split("Result:")[1]
        one_worker_output = one_worker_output.split("\n")


        for worker in workers:
            ok = True
            for i in range(attempts):
                p = subprocess.check_output(["mpirun",
                    "-np",
                    str(worker),
                    "distributed-memory.o",
                    "-p",
                    str(precision),
                    "-a",
                    str(array_size)])
                output = p.decode("utf-8")
                this_output_is_ok = False
                to_process = output.split("Result:")[1]
                # print(to_process)
                to_process = to_process.split("\n")
                # print(to_process)
                for result_line, ref_line in zip(to_process, one_worker_output):
                    result_line = result_line.split(" ")
                    ref_line = ref_line.split(" ")
                    if result_line != ref_line:
                        print(result_line, ref_line)
                        ok = False

            if ok is False:
                results += [[array_size, worker, precision, attempts, "ERROR"]]
            else:
                results += [[array_size, worker, precision, attempts, "OK"]]
            print(f"Done: worker {worker} precision {precision} array \
{array_size}.")

with open("test_output.csv", "a") as file:
    writer = csv.writer(file)
    for row in results:
        writer.writerow(row)

print(results)
