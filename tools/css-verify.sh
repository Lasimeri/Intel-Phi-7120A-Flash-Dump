#!/usr/bin/env bash
# css-verify.sh IMAGE OFFSET [LABEL]
#
# Verify the RSA signature of one Intel CSS module inside IMAGE, starting at OFFSET.
# Module header: 0x18 total size in dwords, 0x80 modulus (256 B), 0x180 exponent,
# 0x184 signature (256 B); all three little-endian. Signature scheme: RSA-2048,
# PKCS#1 v1.5, SHA-256 over header[0:0x80] || module[0x284:size].
# Prints one line: label, module size, key hash, digest, VALID or INVALID.
set -eu

img=$1
mod=$(($2))
label=${3:-$1@$2}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

# le OFFSET LEN: LEN bytes of IMAGE at OFFSET, byte-reversed, as hex
le() { tail -c +$(($1 + 1)) "$img" | head -c "$2" | xxd -p -c1 | tac | tr -d '\n'; }

size=$((0x$(le $((mod + 0x18)) 4) * 4))
printf 'asn1=SEQUENCE:k\n[k]\nn=INTEGER:0x%s\ne=INTEGER:0x%s\n' \
	"$(le $((mod + 0x80)) 256)" "$(le $((mod + 0x180)) 4)" > "$tmp/key.cnf"
openssl asn1parse -genconf "$tmp/key.cnf" -out "$tmp/key.der" > /dev/null
openssl rsa -RSAPublicKey_in -inform DER -in "$tmp/key.der" -pubout -out "$tmp/key.pem" 2> /dev/null
le $((mod + 0x184)) 256 | xxd -r -p > "$tmp/sig"
openssl pkeyutl -verifyrecover -pubin -inkey "$tmp/key.pem" \
	-pkeyopt rsa_padding_mode:none -in "$tmp/sig" -out "$tmp/em"

# 00 01, 202 x FF, 00, SHA-256 DigestInfo, then the 32-byte digest
want_prefix="0001$(printf 'ff%.0s' $(seq 202))003031300d060960864801650304020105000420"
prefix=$(head -c 224 "$tmp/em" | xxd -p -c 224)
signed=$(tail -c 32 "$tmp/em" | xxd -p -c 32)
computed=$({ tail -c +$((mod + 1)) "$img" | head -c 128
	tail -c +$((mod + 0x284 + 1)) "$img" | head -c $((size - 0x284)); } | sha256sum | cut -c1-64)
key=$(sha256sum < "$tmp/key.der" | cut -c1-16)

if [ "$prefix" = "$want_prefix" ] && [ "$signed" = "$computed" ]; then
	result=VALID
else
	result=INVALID
fi
printf '%-34s size=0x%06x key=%s sha256=%s %s\n' "$label" "$size" "$key" "$computed" "$result"
