/** @file main.c
 * 
 * @brief Main file and entry point for the application.
 */

#include "main.h"

#define THIS_FILE__ "main.c"

extern void initialise_monitor_handles(void);
void delay(uint32_t ms);
void gpio_init(void);

extern volatile uint32_t ticks;
/**
 * @brief Main entry point for the application
 */
int
main(void)
{   
    initialise_monitor_handles();
    clock_init();
    usb_driver_t usb_driver = {0};
    usb_init_t usb_init_data = { .vbus_sensing = true };
    usb_init(&usb_driver, &usb_init_data);
    
    for(;;)
    {

    }

    return 0;
}

void
delay(uint32_t ms)
{
    uint32_t start = ticks;
    while(ticks - start < ms)
    {
        // Do nothing
    }
}

/*** end of file ***/