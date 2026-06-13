#!/usr/bin/env python3

import os
import re

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
SCRIPT_HTML = os.path.join(SCRIPT_DIR, "supporters.html")

sum_donations = float(0)
num_supporters = 0

with open(SCRIPT_HTML, "r", encoding="utf-8") as fd:
    for supporter_line in fd:
        # Require the line to start with an asterisk to ignore the header
        match = re.search(r"^\s*\*\s+.*donated €(\d+,\d+|\d+)", supporter_line)
        if match:
            curr_donation = float(match.group(1).replace(",","."))
            sum_donations += curr_donation
            num_supporters += 1
        else:
            print(supporter_line, end="")

print("{} supporters, sum = €{:.2f}\n".format(num_supporters, round(sum_donations, 2)))
