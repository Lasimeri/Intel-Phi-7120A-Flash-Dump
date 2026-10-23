# Reference material

Intel files used as the ground truth for the comparison. Every file is unmodified.

## Sources

| Path | Origin |
| --- | --- |
| `intel-flash-0390/` | `glibc2.12.2pkg-mpss-flash-3.4.2-1` RPM, `/usr/share/mpss/flash/` |
| `rasmm-3.4.2/` | `glibc2.12.2pkg-mpss-rasmm-kernel-3.4.2-1` RPM, `/usr/share/mpss/boot/` |
| `intel-flash-0391/` | `glibc2.12pkg-mpss-flash-3.8.6-1` RPM, `/usr/share/mpss/flash/` |
| `packages/` | The three RPMs above, as shipped |
| `mpss-modules-3.8.6/` | Excerpts of the MPSS 3.8.6 host and card driver source (GPL-2.0) |
| `LICENSE-Intel-MPSS-3.4.2.txt` | `docs/license.txt` from `mpss-3.4.2-linux.tar.gz` |
| `LICENSE-Intel-MPSS-3.8.6.txt` | `docs/license.txt` from `mpss-3.8.6-linux.tar` |

The 3.4.2 RPMs come from `mpss-3.4.2-linux.tar.gz`, downloaded from
`https://downloads.dell.com/folder02776763m/1/mpss-3.4.2-linux.tar.gz` (MD5
`c44f756416e02afc623c45a9b39168e1`, matching the server's Content-MD5). The 3.8.6 files come from
the archive.org item `intel-mpss-3.8.6`.

## Release assets

Attached to the [`reference-tarballs`](../../../releases/tag/reference-tarballs) release because
they exceed GitHub's file size limit:

| File | Size (bytes) | SHA-256 |
| --- | --- | --- |
| `mpss-3.4.2-linux.tar.gz` | 417,388,746 | `2ff04e07fceb9ee9c174a2613851a311b6c823e4a07999401ec497b05fcc7e05` |
| `mpss-src-3.8.6.tar` | 296,581,120 | `2550f10c32cc28ed90049c1fe67eccf4f29a9f2a9991fa0d90f18d86d8db7d70` |

## Flash images

| File | SHA-256 |
| --- | --- |
| `intel-flash-0390/EXT_HP2_C0_0390-02.rom.smc` | `5e2c45ea27d8c4884b7b6c3f972424f40ee32faffaf63357a16e330a06cb9e84` |
| `intel-flash-0391/EXT_HP2_C0_0391-02.rom.smc` | `453d2ab6399c67b517270aa187fe472f457e59bbdb871954b4ffa902e4fc3cbe` |

The A0, B0 and B1 images are for earlier steppings; the 7120A is C0. Hashes of every file are in
`../SHA256SUMS`.
