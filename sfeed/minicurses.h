#undef  OK
#define OK  (0)

const char *clr_eol = "\x1b[K";
const char *clear_screen = "\x1b[H\x1b[2J";
const char *cursor_address = "\x1b[%ld;%ldH";
const char *cursor_normal = "\x1b[?25h"; /* DECTCEM (in)Visible cursor */
const char *cursor_invisible = "\x1b[?25l"; /* DECTCEM (in)Visible cursor */
const char *eat_newline_glitch = (void *)1;
const char *enter_ca_mode = "\x1b[?1049h"; /* smcup */
const char *exit_ca_mode = "\x1b[?1049l"; /* rmcup */
const char *save_cursor = "\x1b""7";
const char *restore_cursor = "\x1b""8";
const char *exit_attribute_mode = "\x1b[0m";
const char *enter_bold_mode = "\x1b[1m";
const char *enter_dim_mode = "\x1b[2m";
const char *enter_reverse_mode = "\x1b[7m";

int
setupterm(char *term, int fildes, int *errret)
{
	return OK;
}

char *
tparm(char *s, long p1, long p2, ...)
{
	static char buf[32];

	if (s == cursor_address) {
		snprintf(buf, sizeof(buf), s, p1 + 1, p2 + 1);
		return buf;
	}

	return s;
}
