#!/bin/bash
set -e

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
IN_ISS="${SCRIPT_DIR}/cherrytree.iss"
OUT_NOLATEX_ISS="${SCRIPT_DIR}/cherrytree_nolatex.iss"
cp ${IN_ISS} ${OUT_NOLATEX_ISS}
sed -b -i 's/_win64_portable/_win64_portable_nolatex/g' ${OUT_NOLATEX_ISS}
sed -b -i 's/_win64_setup/_win64_setup_nolatex/g' ${OUT_NOLATEX_ISS}
