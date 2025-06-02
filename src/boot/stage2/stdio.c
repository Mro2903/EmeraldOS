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

#define DEFAULT_STATE 0
#define FLAGS_STATE 1
#define WIDTH_STATE 2
#define PRECISION_STATE 3
#define LENGTH_STATE 4
#define LONG_STATE 5
#define SHORT_STATE 6
#define SPECIFIER_STATE 7

#define NO_LENGTH 0
#define LONG 1
#define LONG_LONG 2
#define SHORT 3
#define SHORT_SHORT 4

int* printf_number(int* argp, int length, bool sign, int radix);

void __cdecl printf(const char *format, ...)
{
    int *argp = (int *)&format; // Start with the format string
    int state = DEFAULT_STATE; // Initialize the state variable
    int flags; // flags for formatting
    int width; // width for formatting
    int precision; // precision for formatting
    int length; // length modifier
    int radix; // radix for number formatting
    bool sign; // sign for number formatting

    argp++; // Move to the first argument after the format string
    while (*format) {
        switch (state) {
            case DEFAULT_STATE:
                if (*format == '%') {
                    state = FLAGS_STATE; // Move to flags state on '%'
                    format++;
                } else {
                    putc(*format++); // Output regular character
                }
                break;
            case FLAGS_STATE:
                flags = 0; // Reset flags
                while (*format) {
                    switch (*format) {
                        case '-': // Left justify
                            flags |= 0x01; // Set left justify flag
                            format++;
                            break;
                        case '+': // Force sign
                            flags |= 0x02; // Set force sign flag
                            format++;
                            break;
                        case ' ': // Space for positive numbers
                            flags |= 0x04; // Set space flag
                            format++;
                            break;
                        case '#': // Alternate form
                            flags |= 0x08; // Set alternate form flag
                            format++;
                            break;
                        case '0': // Zero padding
                            flags |= 0x10; // Set zero padding flag
                            format++;
                            break;
                        default:
                            goto end_flags; // Exit flags parsing loop
                    }
                }
                end_flags:
                state = WIDTH_STATE;
                break;

            case WIDTH_STATE:
                width = 0; // Reset width
                while (*format >= '0' && *format <= '9') {
                    width = width * 10 + (*format - '0'); // Parse width
                    format++;
                }
                if (*format == '*') { // Width specified by argument
                    width = *(int *)argp; // Get width from argument
                    argp++; // Move to next argument
                    format++;
                }
                if (width < 0) { // If width is negative, treat as left justify
                    flags |= 0x01; // Set left justify flag
                    width = -width; // Make width positive
                }
                if (width > 0) {
                    flags |= 0x20; // Set width flag
                }
                state = PRECISION_STATE;
                break;

            case PRECISION_STATE:
                precision = -1; // Reset precision
                if (*format == '.') { // Precision specified
                    format++; // Skip the '.'
                    precision = 0; // Reset precision
                    while (*format >= '0' && *format <= '9') {
                        precision = precision * 10 + (*format - '0'); // Parse precision
                        format++;
                    }
                    if (precision < 0) {
                        precision = 0; // If negative, set to zero
                    }
                } else if (*format == '*') { // Precision specified by argument
                    precision = *(int *)argp; // Get precision from argument
                    argp++; // Move to next argument
                    format++;
                }
                if (precision < 0) {
                    precision = 0; // If precision is negative, set to zero
                }
                if (precision > 0) {
                    flags |= 0x40; // Set precision flag
                }
                state = LENGTH_STATE;
                break;

            case LENGTH_STATE:
                if (*format == 'l') {
                    state = LONG_STATE; // Long modifier
                    format++;
                } else if (*format == 'h') {
                    state = SHORT_STATE; // Short modifier
                    format++;
                } else {
                    length = NO_LENGTH; // No length modifier
                    state = SPECIFIER_STATE; // No length modifier
                }
                break;

            case LONG_STATE:
                if (*format == 'l') {
                    length = LONG_LONG; // Long long modifier
                    format++;
                } else {
                    length = LONG; // Just long modifier
                }
                state = SPECIFIER_STATE; // Move to specifier state
                break;
            case SHORT_STATE:
                if (*format == 'h') {
                    length = SHORT_SHORT; // Short short modifier
                    format++;
                } else {
                    length = SHORT; // Just short modifier
                }
                state = SPECIFIER_STATE; // Move to specifier state
                break;

            case SPECIFIER_STATE:
                switch (*format) {
                    case 'c': // Character
                        putc((char)*argp); // Output character
                        argp++; // Move to next argument
                        break;
                    case 's': // String
                        puts((const char *)*argp); // Output string
                        argp++; // Move to next argument
                        break;
                    case '%': // Percent sign
                        putc('%'); // Output percent sign
                        break;
                    case 'd': // Decimal integer
                    case 'i': // Integer
                        radix = 10; // Set radix for decimal
                        sign = true; // Set sign for decimal
                        goto print_number; // Go to number formatting
                    case 'u': // Unsigned integer
                        radix = 10; // Set radix for unsigned decimal
                        sign = false; // No sign for unsigned
                        goto print_number; // Go to number formatting
                    case 'x': // Hexadecimal
                    case 'X': // Hexadecimal (uppercase)
                    case 'p': // Pointer (treated as hexadecimal)
                        radix = 16; // Set radix for hexadecimal
                        sign = false; // No sign for hexadecimal
                        goto print_number; // Go to number formatting
                    case 'o': // Octal
                        radix = 8; // Set radix for octal
                        sign = false; // No sign for octal
                        print_number:
                        argp = printf_number(argp, length, sign, radix); // Format number
                        break;
                    default:
                        break; // Unsupported specifier, just ignore it
                }
                format++;
                state = DEFAULT_STATE; // Reset to default state
                break;
            default:
                break; // Should not reach here, but just in case
        }
    }
}

const char g_HexChars[] = "0123456789abcdef";

int* printf_number(int* argp, int length, bool sign, int radix) {
    unsigned long long value;
    char buffer[32]; // Buffer to hold the formatted number
    char *ptr = buffer + sizeof(buffer); // Pointer to the end of the buffer
    int is_negative = 0; // Flag for negative numbers

    // Determine the value based on length modifier
    switch (length) {
        case LONG:
            if (sign) {
                long n = *(long *)argp; // Get long int value
                if (n < 0) {
                    is_negative = 1; // Set negative flag
                    n = -n; // Convert to positive for formatting
                }
                value = (unsigned long long)n; // Convert to unsigned long
            } else {
                value = (unsigned long)(*(unsigned long *)argp); // Get unsigned long value
            }
            argp += 2; // Move to the next argument (long is 2 int-sized words)
            break;
        case LONG_LONG:
            if (sign) {
                long long n = *(long long *)argp; // Get long long int value
                if (n < 0) {
                    is_negative = 1; // Set negative flag
                    n = -n; // Convert to positive for formatting
                }
                value = (unsigned long long)n; // Convert to unsigned long long
            } else {
                value = *(unsigned long long *)argp; // Get unsigned long long value
            }
            argp += 4; // Move to the next argument (long long is 4 int-sized words)
            break;
        case SHORT:
        case SHORT_SHORT:
        case NO_LENGTH:
            if (sign) {
                int n = *(int *)argp; // Get int value
                if (n < 0) {
                    is_negative = 1; // Set negative flag
                    n = -n; // Convert to positive for formatting
                }
                value = (unsigned long long)n; // Convert to unsigned long long
            } else {
                value = (unsigned int)(*(unsigned int *)argp); // Get unsigned int value
            }
            argp++; // Move to the next argument
            break;
    }

    // Format the number into the buffer
    do {
        uint32_t digit;
        x86_div64_32(value, radix, &value, &digit); // Divide value by radix
        *--ptr = g_HexChars[digit]; // Store the character in the buffer
    } while (value > 0); // Continue until all digits are processed
    if (sign && is_negative) {
        *--ptr = '-'; // Add negative sign if needed
    }

    while (*ptr) {
        putc(*ptr++); // Output each character from the buffer
    }
    

    return argp; // Return the updated argument pointer
}
