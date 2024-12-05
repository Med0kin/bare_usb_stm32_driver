/** @file qassert.c
 * 
 * @brief File containing the implementation of the custom assertion function.
 */

#include "qassert.h"

/**
 * @brief Assertion function.
 * 
 * This function is called when an assertion fails.
 * 
 * @param file_ Name of the file where the assertion failed.
 * @param line_ Line number where the assertion failed.
 */
void
on_assert__(char const * const file_, int line_)
{
    //printf("Assertion failed in file %s, line %d\n", file_, line_);
    __disable_irq();
    
    __BKPT(0);
    
    for(;;)
    {

    }
}