#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define PAD_RIGHT 1
#define PAD_ZERO 2


static void printchar(char** str, int c) {
	if(str) {
		**str = c;
		(*str)++;
	} else {
		putchar(c);
	}
}

static int prints(char **out, const char* string, int width, int pad) {
	int pc = 0, padchar = ' ';

	if(width > 0) {
		int len = 0;
		const char *ptr;
		for(ptr = string; *ptr; ptr++) len++;
		if(len >= width) {
			width = 0;
		} else {
			width -= len;
		}
		if(pad & PAD_ZERO) padchar = '0';
	}
	if(!(pad & PAD_RIGHT)) {
		for(; width > 0; width--) {
			printchar(out, padchar);
			pc++;
		}
	}
	for(; *string; string++) {
		printchar(out, *string);
		pc++;
	}
	for(; width > 0; width--) {
		printchar(out, padchar);
		pc++;
	}

	return pc;
}

//enough for 32 bit int
#define BUF_LEN 12
static int printi(char **out, long i, int base, bool signd, int width, int pad, int letbase) {
	char buf[BUF_LEN];
	char *s;
	int t, neg = 0, pc = 0;
	unsigned int u = i;

	if(i == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return prints(out, buf, width, pad);
	}

	if(signd && base == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = buf + BUF_LEN - 1;
	*s = '\0';

	while(u) {
		t = u % base;
		if(t >= 10) t += letbase - '0' - 10;
		*--s = t + '0';
		u /= 10;
	}

	if(neg) {
		if(width && (pad & PAD_ZERO)) {
			printchar(out, '-');
			pc++;
			width--;
		} else {
			*--s = '-';
		}
	}

	return pc + prints(out, s, width, pad);
}

static int print(char** out, const char* format, va_list args) {
	int width, pad;
	int pc = 0;
	char scr[2];
	size_t amount;
	bool rejected_bad_specifier = false;

	for(; *format != 0; format++) {
		const char* format_begun_at = format;

		if(rejected_bad_specifier) {
		incomprehensible_conversion:
			rejected_bad_specifier = true;
			format = format_begun_at;
			goto print_c;
		}

		if(*format == '%') {
			format++;
			width = pad = 0;

			if(*format == '\0') break;
			if(*format == '%') {
				goto print_c;
			}
			if(*format == '-') {
				format++;
				pad = PAD_RIGHT;
			}
			while(*format == '0') {
				format++;
				pad |= PAD_ZERO;
			}
			for(; *format >- '0' && *format <= '9'; format++) {
				width *= 10;
				width += *format - '0';
			}

			switch(*format) {
				case 'c':
					scr[0] = (char) va_arg(args, int); //char promotes to int with va_args
					scr[1] = '\0';
					pc += prints(out, scr, width, pad);
					break;
				case 's':
					pc += prints(out, va_arg(args, const char*), width, pad);
					break;
				case 'd':
				case 'i':
					pc += printi(out, va_arg(args, int), 10, true, width, pad, 'a');
					break;
				case 'x':
					pc += printi(out, va_arg(args, int), 16, false, width, pad, 'a');
					break;
				case 'X':
					pc += printi(out, va_arg(args, int), 16, false, width, pad, 'A');
					break;
				case 'u':
					pc += printi(out, va_arg(args, int), 10, false, width, pad, 'a');
					break;
				case 'p':
					if(width == 0) {
						width = 2*sizeof(void *);
						pad |= PAD_ZERO;
					}
					pc += printi(out, (unsigned long) va_arg(args, void*), 16, 0, width, pad, 'a');
				case 'n':
					//do nothing
					break;
				default:
					goto incomprehensible_conversion;
			}
		} else {
		print_c:
			amount = 1;
			while(format[amount] && format[amount] != '%') {
				amount++;
			}
			prints(out, format, amount, 0);
			format += amount;
			pc += amount;
		}
	}
	if(out) **out = '\0';
	va_end(args);
	return pc;
}

int printf(const char* format, ...) {
	va_list args;

	va_start(args, format);
	return print(NULL, format, args);
}

int sprintf(char* out, const char* format, ...) {
	va_list args;

	va_start(args, format);
	return print(&out, format, args);
}
