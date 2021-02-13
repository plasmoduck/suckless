/*
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                    Version 2, December 2004
 *
 * Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
 *
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>

static bool newline = false;/* print a new line after each step */
static bool loop = false;   /* wether to loop text or not */
static float delay = 0.1;   /* scroll speed, in seconds */
static int number = 20;     /* number of chars to be shown at the same time */

/* return the len of an utf-8 character */
int utf8_len(unsigned char c)
{
    return c<192 ? 0 : c<224 ? 1 : c<240 ? 2 : 3;
}

/* scroll <input> to stdout */
void skroll (const char *input)
{
    int offset = 0;

    /* main loop. will loop forever if run with -l */
    do
    {
        /*
         * each step of the loop will print the buffer, one byte further after
         * each step. using a carriage return, it makes the text scroll out.
         * leading/ending spaces are here to make sure that the text goes from
         * far right, and goes all the way to far left
         */
        for (offset = 0; input[offset + number] != 0; offset++)
        {
            /* increase the message's length in case of utf-8 chars */
            number += utf8_len(input[offset + number - 1]);

            /* print out `number` characters from the buffer ! */
            write(1, input + offset, number);

            /* if we want a new line, do it here, otherwise, carriage return */
            putc(newline ? '\n' : '\r', stdout);

            /* flush stdout, and wait for the next step */
            fflush(stdout);

            /* decrease length when utf-8 chars disappear to the left */
            number -= utf8_len(input[offset]);
            offset += utf8_len(input[offset]);

            usleep(delay*1000000);
        }
    /* magnolia ? FOWEVA ! */
    } while(loop);

    putc('\b', stdout);

    return; /* void */
}
 
/* returns  char that contains the input bufferized */
const char *bufferize (FILE *stream)
{
    int len = 0;
    char *eol, *buf = NULL;

    /* allocate space to store the input */
    if (!(buf = calloc (LINE_MAX + 1, sizeof(char)))) { return NULL; }
    memset(buf, ' ', LINE_MAX);
    buf[LINE_MAX] = 0;

    /* OMG, NO MORE SPACE LEFT ON DEVICE (or no more input, in fact) */
    if (feof(stream) || !fgets(buf + number, LINE_MAX, stream))
    {
        free (buf);
        return NULL;
    }

    /*
     * we need to remove trailings \n and \0 from input string to sanitize output.
     * the buffer should now look like this:
     * [          my input          \0           \0] 
     *  |         |       |         |             `- last \0, to prevent segfaults
     *  |         |       |          `- remaining spaces (up to LINE_MAX)
     *  |         |        `- trailing spaces, to make the text croll to far left
     *  |          `- the input itself, with \n and \0 removed from it
     *   `- leading spaces, to make the text scroll from far right
     */

    /* get the size of the input (and thus, the position of the \0) */
    len = strnlen(buf, LINE_MAX);
    buf[len] = ' ';

    /* terminate the string a bit further */
    buf[len + number] = 0;

    /* remove those silly \n from the input */
    if ((eol = strchr(buf, '\n')) != NULL) {
        eol[0] = ' ';
    }

    return buf;
}

int main (int argc, char **argv)
{
    char ch;
    const char *buf = NULL;

    while ((ch = getopt(argc, argv, "hd:ln:r")) != -1)
    {
        switch (ch)
        {
            case 'h':
                printf("usage: %s [-hlr] [-d delay] [-n number]\n", argv[0]);
                exit(0);
                break;
            case 'd': delay = strtof(optarg, NULL); break;
            case 'n': number = strtoul(optarg, NULL, 10); break;
            case 'l': loop = true; break;
            case 'r': newline = true; break;
        }
    }

    /* SCROLL ALL THE TEXT! */
    while((buf = bufferize(stdin)) != NULL)
    {
        skroll(buf);
    }

    /* End with a new line, no matter what */
    putc('\n', stdout);

    return 0;
}
