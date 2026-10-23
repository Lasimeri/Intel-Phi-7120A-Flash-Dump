# Method

## Environment

- Coprocessor: Intel Xeon Phi 7120A, C0 stepping, PCI 8086:225C subsystem 7D99, 61 cores, 16 GB GDDR5.
- Coprocessor OS: Linux 7.2.3 from [Lasimeri/Intel-Phi-7120A](https://github.com/Lasimeri/Intel-Phi-7120A),
  booted over PCIe from the host. Kernel configuration: `CONFIG_DEVMEM=y`,
  `CONFIG_STRICT_DEVMEM=y`, `CONFIG_IO_STRICT_DEVMEM` not set.
- Tools on the coprocessor: BusyBox 1.37.0 `dd` and `devmem`.
- Date of the read: 2026-10-23.

## Locating the flash

The MPSS RAS kernel module maps the flash from the coprocessor side, below 4 GiB
(`ras/micras_knc.c` lines 971-976): `KNC_FLASH_TAB` 0xFFF76000 (correctable machine-check filter
at +0x400) and `KNC_FLASH_BASE` 0xFFFA8000 (fboot1 version string at +0x1274, backup at +0x02B8).
The coprocessor's `/proc/iomem` lists System RAM at 0x11000000-0xFEDFFFFF and
0x100000000-0x3DFFFEFFF; 0xFEE00000-0xFFFFFFFF is not claimed by any driver, so
`STRICT_DEVMEM` permits reads there.

## Reads

All reads were plain loads; no flash controller register was written and no flash command was
issued.

| Read | Command on the coprocessor | Size | Time |
| --- | --- | --- | --- |
| Version string probe | `devmem 0xFFFA9274 32` (4 words) | 16 B | |
| Cross-check, read() | `dd if=/dev/mem bs=4096 skip=$((0xFFFA8)) count=2` | 8 KiB | 0.003995 s |
| Cross-check, mmap | `devmem ADDR 32` for every word of 0xFFFA8000-0xFFFA9FFF | 8 KiB | |
| First window read | `dd if=/dev/mem bs=4096 skip=$((0xFFF70)) count=$((0x90))` | 576 KiB | 0.226876 s |
| Full window read | `dd if=/dev/mem bs=4096 skip=$((0xFFE00)) count=$((0x200))` | 2 MiB | 0.273940 s |

Checks on the reads:

- The 8 KiB at 0xFFFA8000 read through `read()` (dd) and through uncached 32-bit loads (busybox
  `devmem` maps `/dev/mem` with `O_SYNC`) are identical. Both are in `dump/crosscheck/`.
- The top 576 KiB of the full read is identical to the first window read taken earlier.
- The four words at 0xFFFA9274 decode to "2.1.02.0390", the version string the RAS module reads.

`micbaseaddressdefine.h` also lists SPI regions just below 64 GiB (0xFFFFF0000, 0xFFFFE0000,
0xFFFFDC000). Single 32-bit reads at 0xFFFFE0000, 0xFFFFF0000, 0xFFFFFFFF0 and 0xFFFFDC000 all
returned 0xFFFFFFFF, while 0xFFFE0000, 0xFFFF0000 and 0xFFFFFFF0 below 4 GiB hold data. That
range is not a flash decode on this coprocessor and was not read further.

## Mapping the image onto the window

The Intel image is not laid out linearly in the window. `tools/align.c` located each 256-byte block
of the dump inside the 0391 image (`results/align-vs-0391.txt`), which exposed the image header copy
at 0xFFFA8000 and the fboot1 module at 0xFFFE0000. The image body begins with a descriptor whose
directory gives each section's size, image offset and address (docs/flash-layout.md);
`tools/dirchk.c` compares the dump with an image entry by entry using that directory.

## Signature checks

`tools/css-verify.sh` recovers the RSA signature of a CSS module with the public key stored in the
module header (`openssl pkeyutl -verifyrecover`, no padding), checks the PKCS#1 v1.5 SHA-256
DigestInfo prefix, and compares the embedded digest with SHA-256 over header bytes 0x00-0x7F and the
module body from 0x284. The scheme was established on Intel's own images, where all eight modules
verify. A one-byte change in the fboot1 body, or a zeroed signature byte, reports INVALID.
