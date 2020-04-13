#!/usr/bin/env python3

import os
import re

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
SCRIPT_HTML = os.path.join(SCRIPT_DIR, "supporters.html")

sum_donations = float(0)
countries_dict = {}
with open(SCRIPT_HTML, "r", encoding="utf-8") as fd:
    for supporter_line in fd:
        match = re.search(r"\(([^\)]+)\) donated €(\d+,\d+|\d+)", supporter_line)
        if match:
            curr_country = match.group(1)
            curr_donation = float(match.group(2).replace(",","."))
            sum_donations += curr_donation
            if not curr_country in list(countries_dict.keys()):
                countries_dict[curr_country] = float(0)
            countries_dict[curr_country] += curr_donation
        else:
            print(supporter_line)
print(sum_donations)
while countries_dict:
    curr_max_key = None
    for curr_key, curr_val in countries_dict.items():
        if not curr_max_key:
            curr_max_key = curr_key
        elif curr_val > countries_dict[curr_max_key]:
            curr_max_key = curr_key
    print(curr_max_key, countries_dict[curr_max_key])
    del countries_dict[curr_max_key]
