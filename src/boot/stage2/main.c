#include "stdint.h"
#include "stdio.h"


void __cdecl cstart_(uint16_t bootDrive) {
    puts("Hello world from C!\r\n");
    for (;;);
}
