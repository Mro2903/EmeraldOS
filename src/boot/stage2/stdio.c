#include "stdio.h"
#include "x86.h"

void __cdecl putc(char c)
{
    // Write the character to the video memory using the x86_Video_WriteCharTeletype function
    x86_Video_WriteCharTeletype(c, 0);
}

void __cdecl puts(const char *s)
{
    // Iterate through the string and write each character using putc
    while (*s) {
        putc(*s++);
    }
}
