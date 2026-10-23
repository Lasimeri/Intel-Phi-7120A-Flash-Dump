# Flash layout

Derived from Intel's `EXT_HP2_C0_0390-02.rom.smc` and `EXT_HP2_C0_0391-02.rom.smc` and from the
coprocessor read. Field meanings marked "inferred" come from the data, not from Intel documentation.

## `.rom.smc` file

Each 1,312,008-byte file is two signed CSS blobs back to back:

| File offset | Size | Content |
| --- | --- | --- |
| 0x000000 | 0x110284 | Flash image: 0x284-byte CSS header + 0x110000-byte body |
| 0x110284 | 0x030284 | SMC firmware (same size as `EXT_HP2_SMC_Bootloader_1_8_4326.css_ab`, 197,252 bytes) |

The flash image body contains two further signed modules: FB1 (fboot1) at file offset 0x070284 and
XFW (RAS maintenance) at 0x090284.

## CSS header

Field names follow the usual Intel CSS header layout; the values are those found in these images.

| Offset | Size | Field |
| --- | --- | --- |
| 0x000 | 4 | Module type, 6 |
| 0x004 | 4 | Header length in dwords, 0xA1 (0x284 bytes) |
| 0x008 | 4 | Header version, 0x10000 |
| 0x010 | 4 | Vendor, 0x8086 |
| 0x014 | 4 | Date, BCD (0x20130910 for 0390, 0x20141204 for 0391) |
| 0x018 | 4 | Total module size in dwords |
| 0x01C | 4 | Key size in dwords, 0x40 |
| 0x020 | 4 | Modulus size in dwords, 0x40 |
| 0x024 | 4 | Exponent size in dwords, 1 |
| 0x080 | 256 | RSA modulus, little-endian |
| 0x180 | 4 | RSA exponent, 0x11 |
| 0x184 | 256 | Signature, little-endian |

Signature: RSA-2048, PKCS#1 v1.5 with a SHA-256 DigestInfo, over header bytes 0x000-0x07F
followed by module bytes 0x284 to the end. Every module in both images and the card's fboot1 use
the key whose DER encoding hashes (SHA-256) to `1d32aec2f70c6283...`.

## Descriptor and directory

The flash image body starts with a descriptor (magic `55 AA FF 00`, a version block, and the
version string "2.1.02.0390" at body offset 0x34). From body offset 0x4C it holds 16-byte
directory records:

```
tag[3] flags[1] | u32 size | u32 body offset | u32 address
```

| Tag | Flags | Size | Body offset | Address | In window |
| --- | --- | --- | --- | --- | --- |
| SLF | 0x02 | 0x70000 | 0x10000 | 0x90000 | no |
| SLR | 0x22 | 0x1000 | 0x4000 | 0x5000 | no |
| SFX | 0x0A | 0x80000 | 0x100000 | 0x180000 | no |
| SRX | 0x2A | 0x3000 | 0x8000 | 0xC000 | no |
| DSC | 0x04 | 0x2000 | 0x2000 | 0x0 | no |
| RPR | 0xA1 | 0x1000 | 0x4000 | 0xFFF75000 | yes |
| MFG | 0x05 | 0x1000 | 0x6000 | 0xFFF76000 | yes |
| PSM | 0x05 | 0x1000 | 0x7000 | 0xFFF77000 | yes |
| XRR | 0xA9 | 0x3000 | 0x8000 | 0xFFF78000 | yes |
| RS1 | 0x05 | 0x1000 | 0xB000 | 0xFFF7B000 | yes |
| PST | 0x05 | 0x1000 | 0xF000 | 0xFFF7F000 | yes |
| PCI | 0x81 | 0x8000 | 0x10000 | 0xFFF80000 | yes |
| PCC | 0x85 | 0x8000 | 0x18000 | 0xFFF88000 | yes |
| GD1 | 0x81 | 0x18000 | 0x20000 | 0xFFF90000 | yes |
| CSS | 0x81 | 0x2000 | 0x38000 | 0xFFFA8000 | yes |
| GCT | 0x85 | 0x4000 | 0x3A000 | 0xFFFAA000 | yes |
| GQT | 0x81 | 0x2000 | 0x3E000 | 0xFFFAE000 | yes |
| PGT | 0x81 | 0x10000 | 0x40000 | 0xFFFB0000 | yes |
| VBL | 0x81 | 0x10000 | 0x50000 | 0xFFFC0000 | yes |
| VBU | 0x81 | 0x10000 | 0x60000 | 0xFFFD0000 | yes |
| FB1 | 0x81 | 0x10000 | 0x70000 | 0xFFFE0000 | yes |
| FB0 | 0x05 | 0x10000 | 0x80000 | 0xFFFF0000 | yes |
| XFW | 0x88 | 0x80000 | 0x100000 | 0x90000 | no |

Notes:

- For in-window entries the address is the coprocessor physical address: every entry that is not
  blank in the image matches the read at that address, apart from the per-board bytes listed in
  docs/results.md.
- For SLF, SLR, SFX, SRX, DSC and XFW the address field is small and its meaning is not established
  (inferred: offsets in the SPI part). The XFW record lists body offset 0x100000, but the signed XFW
  module sits at body offset 0x90000 in both images.
- CSS at 0xFFFA8000 holds a copy of the image's own CSS header (the first 0x2000 bytes of the
  `.rom.smc` file), not the body bytes at offset 0x38000, which are blank in the image.
- FB0 is blank (0xFF) in the image. On the coprocessor this range reads as non-flash content,
  consistent with the on-die fboot0 ROM.

## Window map (0xFFE00000-0xFFFFFFFF)

| Range | Content on this coprocessor |
| --- | --- |
| 0xFFE00000-0xFFF73FFF | 0xFF |
| 0xFFF74000-0xFFF74FFF | 0x00, not covered by any 0390 directory entry |
| 0xFFF75000-0xFFFEFFFF | Directory entries RPR through FB1 |
| 0xFFFF0000-0xFFFFFFFF | FB0 range (fboot0 ROM) |
