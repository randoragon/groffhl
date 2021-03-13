# groffhl - a simple groff highlighter

groffhl is a small program for converting Linux "truecolor" escape sequences into native groff format.
You can then take the output and paste it into another groff document.

## How it works

groffhl intakes a file with colors encoded as 24-bit "truecolor" escape sequences (e.g. `\033[38;2;255;0;0m  *red* \033[m`).
It scans the whole file, remembers every distinct color and substitutes it to groff format (e.g. `\m[red] *red* \m[]`) and
finally prints a header including all color declarations, followed by the formatted code.

## Example

Let's say we want to colorize this code snippet in groff:

![example1](https://raw.githubusercontent.com/Randoragon/groffhl/master/example-1.png)

1. First, we need it as a plaintext file, let's call it "hello.c".
2. Next we can use a tool like [highlight](http://www.andre-simon.de/doku/highlight/en/highlight.php) to do the highlighting:

![example2](https://raw.githubusercontent.com/Randoragon/groffhl/master/example-2.png)

3. Next, we feed this new "highlighted.txt" file to groffhl:

```sh
$ groffhl highlighted.txt >highlighted.groff
```

The resulting `highlighted.groff` file looks like this:

```groff
.defcolor groffhl_0 rgb 0.058824f 0.576471f 0.058824f
.defcolor groffhl_1 rgb 0.325490f 0.988235f 0.988235f
.defcolor groffhl_2 rgb 0.411765f 0.780392f 0.537255f
.defcolor groffhl_3 rgb 1.000000f 1.000000f 1.000000f
.defcolor groffhl_4 rgb 0.776471f 0.254902f 0.776471f
.defcolor groffhl_5 rgb 0.972549f 0.819608f 0.819608f
.defcolor groffhl_6 rgb 0.815686f 0.815686f 0.270588f
.defcolor groffhl_7 rgb 0.901961f 0.298039f 0.901961f

\m[groffhl_0]\m[]\m[groffhl_1]#include <stdio.h>\m[]
\m[groffhl_0]\m[]
\m[groffhl_0]\m[]\m[groffhl_2]int\m[] \m[groffhl_0]\m[]\m[groffhl_3]main\m[]\m[groffhl_0]\m[]\m[groffhl_3](\m[]\m[groffhl_0]\m[]\m[groffhl_2]int\m[] \m[groffhl_0]argc\m[]\m[groffhl_3],\m[] \m[groffhl_0]\m[]\m[groffhl_2]char\m[] \m[groffhl_0]\m[]\m[groffhl_3]**\m[]\m[groffhl_0]argv\m[]\m[groffhl_3])\m[]
\m[groffhl_0]\m[]\m[groffhl_3]{\m[]
\m[groffhl_0]\m[]    \m[groffhl_3]printf\m[]\m[groffhl_0]\m[]\m[groffhl_3](\m[]\m[groffhl_0]\m[]\m[groffhl_4]"Hello, world!\m[]\m[groffhl_5]\en\m[]\m[groffhl_4]"\m[]\m[groffhl_0]\m[]\m[groffhl_3]);\m[]
\m[groffhl_0]\m[]    \m[groffhl_6]return\m[] \m[groffhl_0]\m[]\m[groffhl_7]0\m[]\m[groffhl_0]\m[]\m[groffhl_3];\m[]
\m[groffhl_0]\m[]\m[groffhl_3]}\m[]\m[groffhl_0]\m[]
```
As you can see, groffhl keeps track of all colors and neatly organizes them in a simple declaration block. Also, be wary that `highlight`
produces output intended for terminal emulators which typically have a black background. Meanwhile, groff documents tend to print text
on white pages, so you will likely need to tweak some of the color definitions to suit your needs. For example, in the above file you'll
notice that `groffhl_3` has an RGB value of (1, 1, 1), which is pitch white. On a white page, all code written in this color would
appear invisible, so usually you want to change it manually to black instead.

Here is the final result, after a few small manual tweaks to the color definitions:

![example3](https://raw.githubusercontent.com/Randoragon/groffhl/master/example-3.png)

## Installing

Clone the repo and run the following (if necessary, as root):

    make install

The executable will be installed to `/usr/local/bin/groffhl`.
