/*
 * align: locate each CHUNK-byte block of a flash window dump inside a reference
 * image, without any knowledge of the flash layout.
 *
 * usage: align DUMP BASE_HEX REF [CHUNK]
 *
 * Prints runs of consecutive blocks: "phys range  file range  match" for blocks
 * found at a constant offset in REF, "NO MATCH" for blocks absent from REF, and
 * "uniform" for blocks of a single repeated byte. This is how the image-to-flash
 * mapping was first found, before the flash directory was decoded (see dirchk).
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *slurp(const char *p, long *n)
{
	FILE *f = fopen(p, "rb");
	if (!f) { perror(p); exit(1); }
	fseek(f, 0, SEEK_END); *n = ftell(f); fseek(f, 0, SEEK_SET);
	unsigned char *b = malloc(*n);
	if (fread(b, 1, *n, f) != (size_t)*n) { perror("read"); exit(1); }
	fclose(f);
	return b;
}

static int uniform(const unsigned char *p, int n)
{
	for (int i = 1; i < n; i++) if (p[i] != p[0]) return 0;
	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 4) { fprintf(stderr, "usage: align DUMP BASE_HEX REF [CHUNK]\n"); return 2; }
	long dn, rn;
	unsigned char *d = slurp(argv[1], &dn), *r = slurp(argv[3], &rn);
	unsigned long base = strtoul(argv[2], 0, 16);
	int c = argc > 4 ? atoi(argv[4]) : 256;
	long run_p = -1, run_f = 0, run_end = 0; int run_kind = -1; /* 0 match, 1 nomatch, 2 uniform */
	long matched = 0, nomatch = 0, unif = 0;
	for (long o = 0; o + c <= dn; o += c) {
		int kind; long fo = -1;
		if (uniform(d + o, c)) { kind = 2; unif += c; }
		else {
			unsigned char *h = memmem(r, rn, d + o, c);
			if (h) { kind = 0; fo = h - r; matched += c; } else { kind = 1; nomatch += c; }
		}
		int cont = run_p >= 0 && kind == run_kind &&
			(kind != 0 || fo == run_f + (o - run_p));
		if (!cont) {
			if (run_p >= 0) {
				if (run_kind == 0) printf("%lx-%lx  file %lx-%lx  match\n", base + run_p, base + run_end, run_f, run_f + (run_end - run_p));
				else printf("%lx-%lx  %s\n", base + run_p, base + run_end, run_kind == 1 ? "NO MATCH" : "uniform");
			}
			run_p = o; run_f = fo; run_kind = kind;
		}
		run_end = o + c;
	}
	if (run_kind == 0) printf("%lx-%lx  file %lx-%lx  match\n", base + run_p, base + run_end, run_f, run_f + (run_end - run_p));
	else printf("%lx-%lx  %s\n", base + run_p, base + run_end, run_kind == 1 ? "NO MATCH" : "uniform");
	printf("bytes: matched %ld, no match %ld, uniform %ld (chunk %d)\n", matched, nomatch, unif, c);
	return 0;
}
