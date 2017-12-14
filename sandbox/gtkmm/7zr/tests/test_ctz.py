
import os
import subprocess
import unittest


SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
SZR_DIR = os.path.dirname(SCRIPT_DIR)
SZR_PATH = os.path.join(SZR_DIR, "p7zr")
CTZ_PATH = os.path.join(SCRIPT_DIR, "7zr.ctz")
CTD_PATH = os.path.join(SCRIPT_DIR, "7zr.ctd")
PASSWORD = "7zr"


class TestCTZ(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_help(self):
        out_txt = subprocess.check_output([SZR_PATH])
        self.assertIn("p7zip Version", out_txt)

    def test_extract(self):
        shell_cmd = (SZR_PATH,
                     "e",
                     "-p"+PASSWORD,
                     "-w"+SCRIPT_DIR,
                     "-bd",
                     "-y",
                     "-o"+SCRIPT_DIR,
                      CTZ_PATH)
        #open("tmp.txt", "w").write(" ".join(shell_cmd))
        out_txt = subprocess.check_output(shell_cmd)
        self.assertTrue(os.path.isfile(CTD_PATH))
