# Builds the analysis tools and re-derives every file in results/ and dump/sections/
# from the dump and the Intel reference images in this repository.
#
#   make           build tools into build/
#   make verify    check SHA256SUMS and confirm results/ and dump/sections/ reproduce
#   make results   regenerate results/ and dump/sections/
#
# Requires a C compiler (cc, gcc, clang or tcc), openssl, xxd, sha256sum, cmp.

CC     ?= cc
CFLAGS ?= -O2 -Wall -Wextra

BUILD   := build
DUMP    := dump/flash-ffe00000-2MiB.bin
BASE    := ffe00000
IMG0390 := reference/intel-flash-0390/EXT_HP2_C0_0390-02.rom.smc
IMG0391 := reference/intel-flash-0391/EXT_HP2_C0_0391-02.rom.smc

.PHONY: all verify results clean

all: $(BUILD)/dirchk $(BUILD)/align

$(BUILD)/%: tools/%.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD):
	mkdir -p $@

results: all
	rm -rf dump/sections && mkdir -p dump/sections
	$(BUILD)/dirchk $(IMG0390) $(DUMP) $(BASE) dump/sections > results/sections-vs-0390.txt
	$(BUILD)/dirchk $(IMG0391) $(DUMP) $(BASE) > results/sections-vs-0391.txt
	$(BUILD)/align $(DUMP) $(BASE) $(IMG0391) 256 > results/align-vs-0391.txt
	tools/signatures.sh > results/signatures.txt

verify: all
	sha256sum --quiet -c SHA256SUMS
	rm -rf $(BUILD)/sections && mkdir -p $(BUILD)/sections
	$(BUILD)/dirchk $(IMG0390) $(DUMP) $(BASE) $(BUILD)/sections | cmp - results/sections-vs-0390.txt
	diff -r $(BUILD)/sections dump/sections
	$(BUILD)/dirchk $(IMG0391) $(DUMP) $(BASE) | cmp - results/sections-vs-0391.txt
	$(BUILD)/align $(DUMP) $(BASE) $(IMG0391) 256 | cmp - results/align-vs-0391.txt
	tools/signatures.sh | cmp - results/signatures.txt
	@echo "verify: all checks reproduced"

clean:
	rm -rf $(BUILD)
