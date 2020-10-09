/*
 * UNG's Not GNU
 *
 * Copyright (c) 2020, Jakob Kaivo <jkk@ung.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _XOPEN_SOURCE 700
#include <errno.h>
#include <locale.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static FILE *comm_open(const char *path)
{
	if (strcmp(path, "-") == 0) {
		return stdin;
	}

	FILE *f = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "comm: %s: %s\n", path, strerror(errno));
		exit(1);
	}
	return f;
}

static FILE *comm_line(FILE *f, char *buf, size_t n)
{
	if (f == NULL) {
		buf[0] = '\0';
	} else if (fgets(buf, n, f) == NULL) {
		buf[0] = '\0';
		fclose(f);
		f = NULL;
	} else {
		char *nl = strchr(buf, '\n');
		if (nl) {
			*nl = '\0';
		}
	}

	return f;
}

static void comm_print(const char *lead, const char *line)
{
	if (lead) {
		printf("%s%s\n", lead, line);
	}
}

static FILE * comm_until(FILE *f[static 2], char *lead[], size_t bufsize, char buf[2][bufsize], int this)
{
	size_t that = (this == 0) ? 1 : 0;

	int coll;
	while ((f[this] != NULL) && ((coll = strcoll(buf[this], buf[that])) < 0)) {
		comm_print(lead[this], buf[this]);
		f[this] = comm_line(f[this], buf[this], bufsize);
	}

	if (coll == 0 && f[0] && f[1]) {
		comm_print(lead[2], buf[this]);
	} else {
		comm_print(lead[that], buf[that]);
	}

	return f[this];
}

static int comm(FILE *f[static 2], char *lead[static 3])
{
	int comp = 0;
	char buf[2][LINE_MAX];

	while (f[0] && f[1]) {
		//if (comp >= 0) {
			f[0] = comm_line(f[0], buf[0], sizeof(buf[0]));
		//}

		//if (comp <= 0) {
			f[1] = comm_line(f[1], buf[1], sizeof(buf[1]));
		//}

		comp = strcoll(buf[0], buf[1]);
		if (comp == 0) {
			comm_print(lead[2], buf[0]);
		} else if (comp < 0) {
			f[0] = comm_until(f, lead, sizeof(buf[0]), buf, 0);
		} else {
			f[1] = comm_until(f, lead, sizeof(buf[1]), buf, 1);
		}
	}

	if (f[0]) {
		f[0] = comm_until(f, lead, sizeof(buf[0]), buf, 0);
	}

	if (f[1]) {
		f[1] = comm_until(f, lead, sizeof(buf[1]), buf, 1);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	char *lead[] = { "", "\t", "\t\t" };

	int c;
	while ((c = getopt(argc, argv, "123")) != -1) {
		switch (c) {
		case '1':
			lead[0] = NULL;
			lead[1] = "";
			if (lead[2]) {
				lead[2]++;
			}
			break;

		case '2':
			lead[1] = NULL;
			if (lead[2]) {
				lead[2]++;
			}
			break;

		case '3':
			lead[2] = NULL;
			break;

		default:
			return 1;
		}
	}

	if (optind < argc - 2) {
		fprintf(stderr, "comm: too many operands\n");
		return 1;
	}

	if (optind >= argc - 1) {
		fprintf(stderr, "comm: missing operands\n");
		return 1;
	}

	FILE *files[2];
	files[0] = comm_open(argv[optind]);
	files[1] =  comm_open(argv[optind + 1]);

	return comm(files, lead);
}
