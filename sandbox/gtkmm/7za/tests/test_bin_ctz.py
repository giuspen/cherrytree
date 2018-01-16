# -*- coding: UTF-8 -*-
import os
import subprocess
import unittest
from lxml import etree


SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
SZA_DIR = os.path.dirname(SCRIPT_DIR)
SZA_PATH = os.path.join(SZA_DIR, "p7za")
CTZ_INPUT_PATH = os.path.join(SCRIPT_DIR, "7zr.ctz")
CTD_TMP_PATH = os.path.join(SCRIPT_DIR, "7zr.ctd")
CTZ_TMP_PATH = os.path.join(SCRIPT_DIR, "7zr2.ctz")
PASSWORD = "7zr"


class TestCTZ(unittest.TestCase):

    def setUp(self):
        self._cleanup()

    def tearDown(self):
        self._cleanup()

    def _cleanup(self):
        for filepath in (CTD_TMP_PATH, CTZ_TMP_PATH):
            if os.path.isfile(filepath):
                os.remove(filepath)

    def _extract(self, input_path):
        shell_cmd = (SZA_PATH,
                     "e",
                     "-p"+PASSWORD,
                     "-w"+SCRIPT_DIR,
                     "-bd",
                     "-y",
                     "-o"+SCRIPT_DIR,
                     input_path)
        #open("tmp.txt", "w").write(" ".join(shell_cmd))
        out_txt = subprocess.check_output(shell_cmd)

    def _archive(self, input_path, ouput_path):
        shell_cmd = (SZA_PATH,
                     "a",
                     "-p"+PASSWORD,
                     "-w"+SCRIPT_DIR,
                     "-mx1",
                     "-bd",
                     "-y",
                     ouput_path,
                     input_path)
        #open("tmp.txt", "w").write(" ".join(shell_cmd))
        out_txt = subprocess.check_output(shell_cmd)

    def test_help(self):
        out_txt = subprocess.check_output([SZA_PATH])
        self.assertIn("p7zip Version", out_txt)

    def test_extract_archive(self):
        self._extract(CTZ_INPUT_PATH)
        self.assertTrue(os.path.isfile(CTD_TMP_PATH))
        xml_txt = open(CTD_TMP_PATH, "r").read()
        root = etree.fromstring(xml_txt)
        self.assertEqual(root.tag, "cherrytree")
        self.assertEqual(root.xpath("node/@name")[0], "NodeName")
        self.assertEqual(root.xpath("node/rich_text/text()")[0], "NodeContent")
        self._archive(CTD_TMP_PATH, CTZ_TMP_PATH)
        self.assertTrue(os.path.isfile(CTZ_TMP_PATH))
        os.remove(CTD_TMP_PATH)
        self._extract(CTZ_TMP_PATH)
        self.assertTrue(os.path.isfile(CTD_TMP_PATH))
        xml_txt_bis = open(CTD_TMP_PATH, "r").read()
        self.assertEqual(xml_txt, xml_txt_bis)
