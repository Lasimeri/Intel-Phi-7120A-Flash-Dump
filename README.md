# Intel Xeon Phi 7120A Flash Dump

SPI flash contents of an Intel Xeon Phi 7120A coprocessor (Knights Corner, C0 stepping,
PCI ID 8086:225C, subsystem 7D99), read in software without JTAG and checked section by
section against Intel's published flash images.

## Summary

| Item | Result |
| --- | --- |
| Flash version | 2.1.02.0390, built 09/10/2013 10:42:33 (the image shipped with Intel MPSS 3.3 and 3.4) |
| Read method | `/dev/mem` on the coprocessor's own Linux, physical window 0xFFE00000-0xFFFFFFFF (2 MiB) |
| fboot1 (second-stage bootstrap, 64 KiB) | Valid Intel RSA-2048 signature; byte-identical to Intel's 0390 image |
| Other in-window flash sections | Identical to Intel 0390 except per-board data: 4,493 bytes in RPR, MFG, PCI, PCC and GCT |
| Intel reference images | All four signed modules of the 0390 and 0391 images verify under the same key |
| Not covered | XFW (RAS maintenance module) and five other directory entries outside the window; SMC firmware |

Details: [docs/results.md](docs/results.md). Method: [docs/method.md](docs/method.md).
Flash format: [docs/flash-layout.md](docs/flash-layout.md).

## Repository layout

```
dump/
  flash-ffe00000-2MiB.bin   read of card physical 0xFFE00000-0xFFFFFFFF (serial redacted)
  sections/                 one file per in-window flash directory entry: <phys>_<tag>.bin
  crosscheck/               0xFFFA8000 + 8 KiB read with dd (read()) and with devmem (32-bit mmap loads)
results/                    output of `make results`: section comparisons, signature checks, chunk alignment
tools/                      dirchk.c, align.c, css-verify.sh, signatures.sh
docs/                       method, flash layout, results
reference/                  Intel flash images 0390 and 0391, RAS maintenance ELFs, packages, MPSS source excerpts
```

The two MPSS archives behind `reference/` (`mpss-3.4.2-linux.tar.gz`, 417,388,746 bytes, and
`mpss-src-3.8.6.tar`, 296,581,120 bytes) exceed GitHub's file size limit and are attached to the
[`reference-tarballs`](../../releases/tag/reference-tarballs) release. Hashes are in
[reference/README.md](reference/README.md).

## Reproducing the analysis

```
make verify
```

`make verify` checks `SHA256SUMS`, builds the tools, regenerates every file in `results/` and
`dump/sections/`, and compares each with the committed copy byte for byte. Requirements: a C
compiler (`cc`, `gcc`, `clang` or `tcc`, selectable with `make CC=...`), `openssl`, `xxd`,
`sha256sum`, `cmp` and `diff`. `make results` rewrites the committed outputs.

## Redaction

The board serial number (12 ASCII bytes at 0xFFFADFF0, inside the GCT section) is replaced with
`XXXXXXXXXXXX` in `dump/flash-ffe00000-2MiB.bin` and `dump/sections/fffaa000_GCT.bin`. No other
byte differs from the device read. The remaining per-board fields in RPR, MFG, PCI, PCC and GCT
are configuration and manufacturing data and may include other board-specific values.

## Limitations

- The memory-mapped window exposes the directory entries whose addresses fall in
  0xFFF75000-0xFFFFFFFF. XFW (Intel's signed RAS maintenance module, a 0x80000-byte slot) and the
  SLF, SLR, SFX, SRX and DSC entries have no address in that window and were not read. MPSS reads
  them through its maintenance-mode flash commands (`host/tools_support.c`, `FLASH_CMD_READ`), which
  this project has not implemented.
- The SMC microcontroller's firmware has no software read path in MPSS; only its version is exposed.
- 0xFFFF0000-0xFFFFFFFF is consistent with the on-die fboot0 ROM rather than flash (see
  docs/results.md). It is included as read.
- A software read passes through the coprocessor's own flash interface. Reading the SPI part with an
  external programmer (SOIC clip and flashrom) is the independent check and also needs no JTAG.

## Licensing

- Tools and documentation: MIT, see [LICENSE.md](LICENSE.md).
- `dump/` holds Intel firmware as read from the device; Intel retains its copyright.
- `reference/`: Intel MPSS files under Intel's license, included beside them
  (`reference/LICENSE-Intel-MPSS-3.4.2.txt`, `reference/LICENSE-Intel-MPSS-3.8.6.txt`); the
  `mpss-modules` source excerpts are GPL-2.0.

## References

- Intel MPSS 3.8.6 `mpss-modules`: `ras/micras_knc.c` lines 971-1210 (flash window used by the RAS
  module), `host/tools_support.c` (maintenance-mode flash commands), `include/mic/io_interface.h`
  (command and status codes), `include/mic/micbaseaddressdefine.h` (coprocessor address map).
  Excerpts in `reference/mpss-modules-3.8.6/`.
- Intel Xeon Phi Coprocessor Developer's Quick Start Guide for MPSS 3.4, Table 1 (flash version per
  MPSS release).
- Coprocessor OS used for the read: [Lasimeri/Intel-Phi-7120A](https://github.com/Lasimeri/Intel-Phi-7120A).
