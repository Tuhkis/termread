#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <zip.h>

#include "config.h"
#include "xml.h"

#define CTRL_KEY(k) ((k) & 0x1f)

typedef enum
{
	APP_STATE_READ,
	APP_STATE_HELP,
	APP_STATE_CONTENTS
} AppState;

typedef struct 
{
	AppState state;
	int chapter_index;
} App;

typedef struct
{
	size_t length;
	size_t pointer;
	size_t bump;
	char *data;
} AppendBuffer;

static struct termios original_termios;
static App app = {0};

void die(const char *s);

void append_buffer_init(AppendBuffer *buffer, size_t size)
{
	buffer->length = size;
	buffer->bump = 64;
	buffer->data = malloc(buffer->length);
	buffer->pointer = 0;
	memset(buffer->data, 0, buffer->length);
}

void append_buffer_append(AppendBuffer *buffer, char c)
{
	if (buffer->pointer >= buffer->length)
	{
		buffer->length += buffer->bump;
		buffer->data = realloc(buffer->data, buffer->length);
	}
	buffer->data[buffer->pointer] = c;
	buffer->pointer++;
}

void append_buffer_write(AppendBuffer *buffer, const char *s, size_t length)
{
	size_t i;
	for (i = 0; i < length; ++i)
	{
		append_buffer_append(buffer, s[i]);
	}
}

void append_buffer_reset(AppendBuffer *buffer)
{
	memset(buffer->data, 0, buffer->length);
	buffer->pointer = 0;
}

void append_buffer_deinit(AppendBuffer *buffer)
{
	free(buffer->data);
}

void get_screen_dimensions(int *width, int *height)
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	*width = w.ws_col;
	*height = w.ws_row;
}

void enter_raw_mode(void)
{
	struct termios raw;
	if (tcgetattr(STDIN_FILENO, &raw) == -1) die("tcgetattr");

	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

void disable_raw_mode(void)
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1) die("tcsetattr");
}

char read_key(void)
{
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) 
		if (nread == 1 && errno != EAGAIN) die("read");
	return c;
}

/* Drawing functions */
void refresh_screen(void)
{
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
}

void show_cursor(void)
{
	write(STDOUT_FILENO, "\x1b[?25h", 6);
}

void ab_hide_cursor(AppendBuffer *ab)
{
	append_buffer_write(ab, "\x1b[?25l", 6);
}

void ab_show_cursor(AppendBuffer *ab)
{
	append_buffer_write(ab, "\x1b[?25h", 6);
}

void ab_refresh_screen(AppendBuffer *ab)
{
	append_buffer_write(ab, "\x1b[2J", 4);
	append_buffer_write(ab, "\x1b[H", 3);
}

void die(const char *s)
{
	show_cursor();
	refresh_screen();
	perror(s);
	exit(1);
}

void shutdown(void)
{
	disable_raw_mode();
	show_cursor();
	refresh_screen();
}

int main(int argc, char **argv)
{
	int running = 1;
	int counter = 0;
	AppendBuffer ab;
	int screen_width = 0;
	int screen_height = 0;
	int i = 0;

	xml_test();
	/* return 0; */

	for (i = 1; i < argc; ++i)
	{
		if (argv[i][0] != '-')
		{
			int errorp = 0;
			char *txt = NULL;
			zip_file_t *fd;
			zip_t *arch = zip_open(argv[i], 0, &errorp);	
			if (arch == NULL)
			{
				fprintf(stderr, "Could not open epub. Perhaps %s is not of the correct filetype?\n", argv[i]);
				return 1;
			}
			zip_stat_t *info = malloc(1024 * 1024);
			zip_stat_init(info);

			zip_stat(arch, "mimetype", 0, info);

			txt = malloc(info->size + 1);
			memset(txt, 0, info->size + 1);
			fd = zip_fopen(arch, "mimetype", 0);
			if (fd == NULL)
			{
				fprintf(stderr, "No mimetype in %s.\n", argv[i]);
				return 1;
			}
			zip_fread(fd, txt, info->size);
			zip_fclose(fd);

			free(info);

			if (strcmp(txt, "application/epub+zip") == 1)
			{
				fprintf(stderr, "Wrong mimetype in %s.\n", argv[i]);
				return 1;
			}
			free(txt);

			/* return 0; TEMPORARY */
		} else
		{
			switch(argv[i][1])
			{
				case 'h':
					printf("TODO: Implement. lol\n");
					return 0;
					break;
				default:
					printf("Unknown flag \"%s\"\nTry \"-h\" for help.\n", argv[i]);
					return 1;
					break;
			}
		}
	}

	if (tcgetattr(STDIN_FILENO, &original_termios) == -1) die("tcgetattr");
	atexit(shutdown);

	enter_raw_mode();

	get_screen_dimensions(&screen_width, &screen_height);
	append_buffer_init(&ab, screen_width * screen_height);
	while (running)
	{
		int calc_left_padding = left_padding == -1 ? (screen_width - text_width) / 2 : left_padding;
		int calc_text_width = text_width > screen_width ? screen_width : text_width;

		append_buffer_reset(&ab);
		ab_hide_cursor(&ab);
		ab_refresh_screen(&ab);

		for (i = 0; i < screen_height; i++)
		{
			int ii = 0;
			if (text_width <= screen_width)
				for (ii = 0; ii < calc_left_padding; ++ii) append_buffer_append(&ab, ' ');

			for (ii = 0; ii < calc_text_width; ++ii) append_buffer_append(&ab, 'a');

			if (i < screen_height -1)
				append_buffer_write(&ab, "\r\n", 2);
		}

		append_buffer_append(&ab, 0);

		write(STDOUT_FILENO, ab.data, ab.length - 1);

		switch(read_key())
		{
			case 'q':
				running = 0;
				break;
			default:
				counter++;
				break;
		}
		get_screen_dimensions(&screen_width, &screen_height);
	}

	append_buffer_deinit(&ab);

	return 0;
}

/* Include other source files for single compile unit builds. */
#include "xml.c"
#include "arena.c"
