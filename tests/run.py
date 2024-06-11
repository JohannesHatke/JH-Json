#! /bin/env python3
from io import TextIOWrapper
import os
import sys
import json

memcheck = True


class colors:
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    END = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def fancy_bool(b: bool) -> str:
    if b:
        return f"{colors.OKGREEN}✓{colors.END}"
    else:
        return f"{colors.FAIL}✗{colors.END}"


def isvalid(filename: str) -> bool:
    with open(filename, "r") as f:
        try:
            json.load(f)
            return True
        except:
            return False


def run_Test_in_Folder(dir: str) -> tuple[bool, bool]:
    global bin_location, failures
    correct_output = True
    leak_output = True
    for file in [os.path.join(dir, x) for x in os.listdir(dir)]:
        result: bool = (os.system(f"{bin_location} {file}") == 0) == isvalid(file)
        correct_output = correct_output and result
        correct_msg = "correct:" + fancy_bool(result)
        leaks = False
        leak_msg = ""
        if memcheck:
            leaks = os.system(f"valgrind --error-exitcode=10 {bin_location} " + file) == 10
            leak_msg = "no leaks:" + fancy_bool(not leaks)
        leak_output = not leaks and leak_output
        if not result or leaks:
            failures.append(file)
        print(f"{colors.UNDERLINE}testing {file} {colors.END} {correct_msg}  {leak_msg}")

    return correct_output, leak_output


test_dir = os.path.dirname(os.path.realpath(__file__))

if len(sys.argv) > 1:
    if sys.argv.pop(1) == "--no-memcheck":
        memcheck = False
    else:
        print("unknown option. only use --no-memcheck")

memcheck = os.system("which valgrind") == 0
bin_location = os.path.realpath(os.path.join(test_dir, "../build/tests"))
subdirs = ["fail_tests", "full_tests", "list_tests", "object_tests", "str_tests"]
test_dirs = [os.path.join(os.path.dirname(os.path.realpath(__file__)), x) for x in subdirs]
failures = []
results = []

if __name__ == "__main__":
    for location in test_dirs:
        print(f"\n\n\n {colors.BOLD}{colors.UNDERLINE} {location} {colors.END}")
        results.append(run_Test_in_Folder(location) )

    print(f"\n\n\n {colors.BOLD}{colors.UNDERLINE} {'TESTS':^40} {colors.END}")
    print(f"{'Folder':<20} {'Correct':^10} {'Memory':^10}")
    for dir,(correct,memsafe) in zip(subdirs,results):
        print(f"{dir:<20}     {fancy_bool(correct)}          {fancy_bool(memsafe)}")

    if failures:
        print(f"\nfound errors in:")
        for f in failures:
            print(f"\t{f}")
