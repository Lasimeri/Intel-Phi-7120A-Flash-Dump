/*
 * dirchk: compare a Xeon Phi x100 flash window dump against an Intel .rom.smc
 * image, one flash directory entry at a time.
 *
 * usage: dirchk IMAGE DUMP DUMP_BASE_HEX [OUTDIR]
 *
 * IMAGE is an Intel CSS blob: a 0x284-byte header, then a body that starts with
 * a descriptor (magic 55 aa ff 00). The descriptor's directory begins at body
 * offset 0x4c and holds 16-byte records:
 *
 *     tag[3] flags[1] | u32 size | u32 body offset | u32 phys
 *
 * Every record whose phys range lies inside the dump is compared byte for byte.
 * The CSS entry is compared against the image's own header, because the flasher
 * stores the signed header there rather than the body bytes at that offset.
 * With OUTDIR, each in-window section of the dump is also written to
 * OUTDIR/<phys>_<tag>.bin.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSS_HDR		0x284
#define DIR_FIRST	0x4c
#define DIR_END		0x1c0
#define DESC_MAGIC	0x00ffaa55u

static unsigned char *slurp(const char *path, long *len)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		perror(path);
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	*len = ftell(f);
	fseek(f, 0, SEEK_SET);
	unsigned char *buf = malloc(*len);
	if (!buf || fread(buf, 1, *len, f) != (size_t)*len) {
		perror(path);
		exit(1);
	}
	fclose(f);
	return buf;
}

static uint32_t le32(const unsigned char *p)
{
	return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24;
}

static long count_ff(const unsigned char *p, long n)
{
	long c = 0;
	for (long i = 0; i < n; i++)
		c += p[i] == 0xff;
	return c;
}

static void write_section(const char *dir, uint32_t phys, const char *tag,
			  const unsigned char *p, long n)
{
	char path[4096];
	snprintf(path, sizeof path, "%s/%08x_%s.bin", dir, phys, tag);
	FILE *f = fopen(path, "wb");
	if (!f || fwrite(p, 1, n, f) != (size_t)n || fclose(f)) {
		perror(path);
		exit(1);
	}
}

int main(int argc, char **argv)
{
	if (argc != 4 && argc != 5) {
		fprintf(stderr, "usage: dirchk IMAGE DUMP DUMP_BASE_HEX [OUTDIR]\n");
		return 2;
	}
	long img_len, dump_len;
	unsigned char *img = slurp(argv[1], &img_len);
	unsigned char *dump = slurp(argv[2], &dump_len);
	uint32_t base = strtoul(argv[3], NULL, 16);
	const char *outdir = argc == 5 ? argv[4] : NULL;
	unsigned char *body = img + CSS_HDR;

	if (img_len < CSS_HDR + DIR_END || le32(body) != DESC_MAGIC) {
		fprintf(stderr, "%s: no flash descriptor at body offset 0\n", argv[1]);
		return 1;
	}
	printf("%-4s %-4s %8s %8s %10s %9s %9s %9s  %s\n", "tag", "flag", "size",
	       "bodyoff", "phys", "diff", "dump_ff", "image_ff", "result");

	long total_diff = 0, compared = 0;
	for (long r = DIR_FIRST; r + 16 <= DIR_END; r += 16) {
		const unsigned char *e = body + r;
		uint32_t size = le32(e + 4), off = le32(e + 8), phys = le32(e + 12);
		if (!size)
			break;
		char tag[4] = { (char)e[0], (char)e[1], (char)e[2], 0 };
		printf("%-4s 0x%02x %8x %8x %10x ", tag, e[3], size, off, phys);
		if (phys < base || (uint64_t)phys + size > (uint64_t)base + dump_len) {
			printf("%9s %9s %9s  outside window\n", "-", "-", "-");
			continue;
		}
		const unsigned char *d = dump + (phys - base);
		const unsigned char *ref = strcmp(tag, "CSS") ? body + off : img;
		if (ref + size > img + img_len) {
			printf("%9s %9s %9s  reference past image end\n", "-", "-", "-");
			continue;
		}
		long diff = 0, first = -1, last = -1;
		for (long i = 0; i < (long)size; i++)
			if (d[i] != ref[i]) {
				diff++;
				if (first < 0)
					first = i;
				last = i;
			}
		printf("%9ld %9ld %9ld  ", diff, count_ff(d, size), count_ff(ref, size));
		if (diff)
			printf("differs at +%lx..+%lx\n", first, last);
		else
			printf("identical\n");
		total_diff += diff;
		compared += size;
		if (outdir)
			write_section(outdir, phys, tag, d, size);
	}
	printf("in-window sections: %ld bytes compared, %ld bytes differ\n", compared, total_diff);
	return 0;
}
