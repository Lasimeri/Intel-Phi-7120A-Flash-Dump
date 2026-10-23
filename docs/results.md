# Results

All numbers below are reproduced by `make verify` from the files in this repository.

## Signatures (`results/signatures.txt`)

| Module | Size | Result |
| --- | --- | --- |
| Intel 0390 outer image, FB1, XFW, SMC firmware | 0x110284, 0x10000, 0x21FD4, 0x30284 | all VALID |
| Intel 0391 outer image, FB1, XFW, SMC firmware | same sizes | all VALID |
| Card FB1 (fboot1) at 0xFFFE0000 | 0x10000 | VALID |

All nine use the same RSA-2048 key. The card's fboot1 digest,
`d5e71697c53ea8a9ea8eb64b0212987d805594bce08dafa1f8cb1d8649bbce67`, equals the digest of Intel's
0390 fboot1.

## Sections against Intel 0390 (`results/sections-vs-0390.txt`)

Seventeen directory entries fall inside the window (557,056 bytes).

| Result | Entries |
| --- | --- |
| Identical, with content | GD1, CSS (signed header copy), FB1 (fboot1), GQT |
| Identical, blank (0xFF) in both | PSM, XRR, RS1, PST, PGT, VBL, VBU |
| Per-board data | RPR, MFG, PCI, PCC, GCT (4,493 bytes in total) |
| Not flash | FB0 (61,271 bytes differ from the blank image range) |

Per-board differences:

| Entry | Bytes | Location | Content |
| --- | --- | --- | --- |
| RPR | 4,096 | whole section | All 0x00 on the card; blank (0xFF) in the image |
| MFG | 250 | +0x400 to +0xFFF | Correctable machine-check filter list, the table MPSS reads at `KNC_FLASH_TAB` + 0x400 (`ras/micras_knc.c` lines 1202-1209); blank in the image |
| PCI | 3 | +0x12, +0x13, +0x40 | +0x12: subsystem ID 0x7004 in the image, 0x7D99 (7120A) on the card. +0x40: 0xFE in the image, 0xFC on the card, meaning not established |
| PCC | 53 | +0x00 to +0x40 | A 0x48-byte block that matches the card's PCI block in all but 4 bytes; blank in the image |
| GCT | 91 | +0x3F80 to +0x3FFF | Board configuration words, the version string "2.1.02.0390" at +0x3FE0 and the board serial at +0x3FF0 (redacted); blank in the image |

The 4 KiB at 0xFFF74000, which no 0390 directory entry covers, is all 0x00 on the card.

## Sections against Intel 0391 (`results/sections-vs-0391.txt`)

The card carries 0390, so the 0391 comparison adds version differences on top of the per-board
data: CSS 284 bytes (header date, signature, version strings), FB1 259 bytes (header date, the
256-byte signature, one character of the version string and one byte of the build number 0x0390
versus 0x0391), PCI 22 bytes.
Apart from those, fboot1 code is the same in 0390 and 0391.

## FB0 range

0xFFFF0000-0xFFFFFFFF does not match the image, which is blank (0xFF) there. The range starts with
the SHA-256 round constants (0x428A2F98, 0x71374491, ...) and holds 61,271 non-0xFF bytes. This is
consistent with the fboot0 boot ROM on the die, which MPSS describes as hardwired in the chip
(`ras/micras_knc.c` line 1189), rather than flash.
