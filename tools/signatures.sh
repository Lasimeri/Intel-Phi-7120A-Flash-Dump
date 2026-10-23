#!/usr/bin/env bash
# signatures.sh: check every CSS module of both Intel reference images and the
# fboot1 module read from the card. Run from the repository root.
set -eu
cd "$(dirname "$0")/.."

verify=tools/css-verify.sh
for ver in 0390 0391; do
	img=reference/intel-flash-$ver/EXT_HP2_C0_$ver-02.rom.smc
	$verify "$img" 0x000000 "Intel $ver outer image"
	$verify "$img" 0x070284 "Intel $ver FB1 (fboot1)"
	$verify "$img" 0x090284 "Intel $ver XFW (RAS maintenance)"
	$verify "$img" 0x110284 "Intel $ver SMC firmware"
done
# 0xFFFE0000 is offset 0x1E0000 in a dump based at 0xFFE00000
$verify dump/flash-ffe00000-2MiB.bin 0x1e0000 "card FB1 (fboot1) at 0xFFFE0000"
