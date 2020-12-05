#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import os
import re
import time
import xml.etree.ElementTree as ET

SCRIPTS_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.dirname(SCRIPTS_DIR)
DATA_DIR = os.path.join(ROOT_DIR, "data")
TEMPLATE_METAINFO_XML_FILEPATH = os.path.join(DATA_DIR, "_com.giuspen.cherrytree.metainfo.xml")
METAINFO_XML_FILEPATH = os.path.join(DATA_DIR, "com.giuspen.cherrytree.metainfo.xml")
DEBIAN_CHANGELOG_PATH = os.path.join(ROOT_DIR, "debian", "changelog")

VERSION = "?"
with open(DEBIAN_CHANGELOG_PATH, "r") as fd:
    for fileline in fd:
        match = re.search(r"cherrytree +\(([0-9]+\.[0-9]+\.[0-9]+)[-+]", fileline)
        if match is not None:
            VERSION = match.group(1)
            #print(VERSION)
            break
DATE = time.strftime(
    "%Y-%m-%d",
    time.gmtime(int(os.environ.get('SOURCE_DATE_EPOCH', time.time())))
)

# <?xml version="1.0" encoding="UTF-8"?>
# <component type="desktop-application">
#  <releases>
#    <release version="0.99.2" date="2020-06-21" />
#  </releases>
# </component>

def main(args):
    tree = ET.parse(TEMPLATE_METAINFO_XML_FILEPATH)
    root = tree.getroot()
    releases = root.findall("./releases")
    assert len(releases) == 1
    releases[0].text = "\n  "
    release_Element = ET.SubElement(releases[0], "release", version=VERSION, date=DATE)
    tree.write(METAINFO_XML_FILEPATH)

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
