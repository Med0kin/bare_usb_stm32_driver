/** @file qassert.h
 * 
 * @brief Custom assertion macros.
 * 
 * @note If you want to know more about Design by Contract I strongly
 *       recommend reading this artickle by BARR group:
 *       https://barrgroup.com/blog/design-contract-embedded-c.
 */

#ifndef QASSERT_H
#define QASSERT_H

#include <stdio.h>

#include "stm32f411xe.h"

void on_assert__(char const * const file_, int line_);

#ifdef NASSERT
 
    #define DEFINE_THIS_FILE
    #define ASSERT(ignore_)             ((void)0)
    #define ALLEGE(test_)               ((void)(test_))

#else
 
    #define DEFINE_THIS_FILE \
        static char const THIS_FILE__[] = __FILE__
 
    #define ASSERT(test_) \
        ((test_) ? (void)0 : on_assert__(THIS_FILE__, __LINE__))
 
    #define ALLEGE(test_)   ASSERT(test_)
 
#endif /* NASSERT */
 
#define REQUIRE(test_)      ASSERT(test_)
#define ENSURE(test_)       ASSERT(test_)
#define INVARIANT(test_)    ASSERT(test_)
 
#endif /* QASSERT_H */

/*** end of file ***/
 