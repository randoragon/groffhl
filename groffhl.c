#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define INIT_BUFSIZE 4096
#define INIT_COLORS 256
#define RGB_SEQ "\033[38;2;"
#define SEQ_BUFSIZE 256
#define NAME_BUFSIZE 16

typedef struct Color Color;

void  convertFile(const FILE *file);
Color extractColor(char *str);
const char *addColor(Color col);
int   allocColors(size_t size);
void  freeColors();
char *stpcpy(char *dest, const char *src); /* C89 does not ship with stpcpy */
int   prepareBuf(char **buffer, size_t *max, size_t needed);
bool  colorEq(Color a, Color b);

struct Color
{
    int r,
        g,
        b;
};

struct {
    Color color;
    char *name;
} *colors        = NULL;
size_t ncolors   = 0;
size_t maxcolors = 0;
const Color reset_color = { 4, 2, 2137 }; /* conventional constant that represents the "\033[m" sequence */

int main(int argc, char *argv[])
{
    allocColors(INIT_COLORS);

    if (argc > 1) {
        int i;

        for (i = 1; i < argc; i++) {
            FILE *file;

            file = fopen(argv[i], "r");
            if (file == NULL) {
                fprintf(stderr, "groffhl: failed to open file \"%s\"\n", argv[i]);
                continue;
            }
            convertFile(file);
            fclose(file);
        }
    } else {
        convertFile(stdin);
    }

    freeColors();

    return 0;
}

void convertFile(const FILE *file)
{
    size_t bufsize;
    char  *output,
          *seq;
    bool   inside_seq;
    int c, i, j;

    bufsize = INIT_BUFSIZE;
    if (!(output = malloc(bufsize * sizeof *output))) {
        fprintf(stderr, "groffhl: malloc failed\n");
        return;
    }
    if (!(seq = malloc(SEQ_BUFSIZE * sizeof *seq))) {
        fprintf(stderr, "groffhl: malloc failed\n");
        free(output);
        return;
    }

    /* Scan file, gradually build its copy with all the
     * appropriate substitutions. The final version cannot
     * be output immediately, because a groff header with
     * declarations of all found colors must be prepended
     * at the end. */
    inside_seq = false;
    i = j = 0;
    while ((c = fgetc((FILE*)file)) != EOF) {
        if (inside_seq) {
            if (i + 1 >= SEQ_BUFSIZE) {
                fprintf(stderr, "groffhl: sequence exceeded SEQ_BUFSIZE\n");
                exit(1);
            }
            seq[i++] = (char)c;
            if (c == 'm') {
                Color col;

                inside_seq = false;
                seq[i] = '\0';

                col = extractColor(seq);
                if (colorEq(col, reset_color)) {
                    const char str[] = "\\m[]";
                    size_t len = strlen(str);
                    if (prepareBuf(&output, &bufsize, j + len)) {
                        exit(1);
                    }
                    strcpy(output + j, str);
                    j += len;
                } else if (col.r < 0 || col.g < 0 || col.b < 0) {
                    /* Escape sequence couldn't be read; treat it
                     * as a string literal and append normally */
                    if (prepareBuf(&output, &bufsize, j + i)) {
                        exit(1);
                    }
                    strncpy(output + j, seq, SEQ_BUFSIZE);
                    j += i;
                } else {
                    const char *name;
                    char *pos;
                    size_t len;

                    if ((name = addColor(col)) == NULL) {
                        fprintf(stderr, "groffhl: addColor failed\n");
                        exit(1);
                    }
                    len = strlen("\\m[]") + strlen(name);
                    if (prepareBuf(&output, &bufsize, j + len)) {
                        exit(1);
                    }
                    pos = output + j;
                    pos = stpcpy(pos, "\\m[");
                    pos = stpcpy(pos, name);
                    pos = stpcpy(pos, "]");
                    j = pos - output;
                }
                seq[0] = '\0';
                i = 0;
            }
        } else {
            if (c == '\033') {
                inside_seq = true;
                seq[0] = (char)c;
                i = 1;
            } else {
                /* Preprocess some characters which groff may interpret non-literally */
                char str[5];
                size_t len;

                switch (c) {
                    case '\\':
                        strcpy(str, "\\e");
                        len = 2;
                        break;
                    default:
                        str[0] = c;
                        str[1] = '\0';
                        len = 1;
                        break;
                }
                if (prepareBuf(&output, &bufsize, j + len)) {
                    exit(1);
                }
                strcpy(output + j, str);
                j += len;
            }
        }
    }
    output[j] = '\0';

    /* Print out the final conversion */
    for (i = 0; i < ncolors; i++) {
        printf(".defcolor %s rgb %ff %ff %ff\n", colors[i].name, colors[i].color.r / 255.0, colors[i].color.g / 255.0, colors[i].color.b / 255.0);
    }
    putchar('\n');
    printf("%s\n", output);

    free(seq);
    free(output);
}

Color extractColor(char *str)
{
    Color ret = { -1, -1, -1 };
    if (sscanf(str, RGB_SEQ"%d;%d;%dm", &ret.r, &ret.g, &ret.b) != 3) {
        if (strcmp(str, "\033[m") == 0) {
            return reset_color;
        }
    }
    return ret;
}

const char *addColor(Color col)
{
    const char prefix[] = "groffhl_";
    int digit_count, max_width;
    int i;

    /* Check if color already exists */
    for (i = 0; i < ncolors; i++) {
        if (colorEq(col, colors[i].color)) {
            return colors[i].name;
        }
    }

    if (ncolors >= maxcolors) {
        if (allocColors(2 * maxcolors)) {
            fprintf(stderr, "groffhl: allocColors failed\n");
            return NULL;
        }
    }

    /* Compute the number of digits in the index number */
    for (digit_count = 1, i = ncolors; i / 10 > 0; digit_count++, i /= 10)
        ;

    /* Compute the maximum acceptable number of digits */
    max_width = NAME_BUFSIZE - strlen(prefix) - 1;

    if (digit_count > max_width) {
        fprintf(stderr, "groffhl: too many distinct colors, color name too long\n");
        return NULL;
    }

    sprintf(colors[ncolors].name, "%s%lu", prefix, ncolors);
    colors[ncolors].color = col;
    return colors[ncolors++].name;
}

/* This function uses realloc to change the maximum number of
 * colors and updates maxcolors accordingly. It can only increase
 * the number, so passing a size lower than maxcolors is undefined.
 */
int allocColors(size_t size)
{
    int i;

    if (!(colors = realloc(colors, size * sizeof *colors))) {
        fprintf(stderr, "groffhl: realloc failed\n");
        return 1;
    }
    for (i = maxcolors; i < size; i++) {
        if (!(colors[i].name = malloc(NAME_BUFSIZE * sizeof *(colors[i].name)))) {
            fprintf(stderr, "groffhl: malloc failed\n");
            return 1;
        }
        colors[i].name[0] = '\0';
    }
    maxcolors = size;
    return 0;
}

void freeColors()
{
    if (colors != NULL) {
        int i;

        for (i = 0; i < maxcolors; i++) {
            free(colors[i].name);
        }
        free(colors);
    }
}

char *stpcpy(char *dest, const char *src)
{
    while ((*dest++ = *src++))
        ;
    return dest - 1;
}

/* Prepares a buffer of size max to be able to store
 * needed size - pretty much just reallocs if necessary
 * and prints a warning. */
int prepareBuf(char **buffer, size_t *max, size_t needed)
{
    if (needed > *max) {
        *max *= 2;
        if (!(*buffer = realloc(*buffer, *max * sizeof **buffer))) {
            fprintf(stderr, "groffhl: prepareBuf realloc failed\n");
            return 1;
        }
    }
    return 0;
}

bool colorEq(Color a, Color b)
{
    return (a.r == b.r) && (a.g == b.g) && (a.b == b.b);
}
