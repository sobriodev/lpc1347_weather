#include "board.h"
#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

int main(void) {
	/* Generic initialization */
    SystemCoreClockUpdate();
    Board_Init();
    Board_LED_Set(0, true);
    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
    }
    return 0 ;
}
