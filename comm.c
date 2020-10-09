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

static FILE * comm_until(FILE *f1, FILE *f2, char *lead[], char s1[], size_t n1, char s2[], size_t n2, int unique)
{
	FILE *f = (unique == 1) ? f1 : f2;
	char *this = (unique == 1) ? s1 : s2;
	char *that = (unique == 1) ? s2 : s1;
	size_t size = (unique == 1) ? n1 : n2;

	int coll;
	while ((f != NULL) && ((coll = strcoll(this, that)) < 0)) {
		comm_print(lead[unique - 1], this);
		f = comm_line(f, this, size);
	}

	if (coll == 0) {
		comm_print(lead[2], this);
	} else {
		size_t l = (unique == 1) ? 1 : 0;
		comm_print(lead[l], that);
	}

	return f;
}

static int comm(FILE *f1, FILE *f2, char *lead[])
{
	int comp = 0;
	char s1[LINE_MAX];
	char s2[LINE_MAX];

	while (f1 && f2) {
		f1 = comm_line(f1, s1, sizeof(s1));

		f2 = comm_line(f2, s2, sizeof(s2));

		comp = strcoll(s1, s2);
		if (comp == 0) {
			comm_print(lead[2], s1);
		} else if (comp < 0) {
			comm_until(f1, f2, lead, s1, sizeof(s1), s2, sizeof(s2), 1);
		} else {
			comm_until(f1, f2, lead, s1, sizeof(s1), s2, sizeof(s2), 2);
		}
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

	FILE *f1 = comm_open(argv[optind]);
	FILE *f2 = comm_open(argv[optind + 1]);

	return comm(f1, f2, lead);
}
