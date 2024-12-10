/** @file usb.h
 * 
 * @brief Header file for usb peripheral.
 */

#ifndef USB_H
#define USB_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "stm32f411xe.h"

#include "qassert.h"


/**
 * @brief USB peripheral states.
 * 
 * The USB peripheral can be in one of the following states.
 * 
 * USB_STATE_NONE:          No state. USB has to be initialized.
 * USB_STATE_POWERED:       USB peripheral is powered and VBUS is present. 
 * USB_STATE_RESET:    Waiting for reset to complete. (Takes ~10 ms) End of reset is
 *                          indicated by ENUMDNE interrupt. Device can only enter this state
 *                          from POWERED state.
 * USB_STATE_DEFAULT:       Default state after reset, at this point EP0 is configured
 *                          and ready to receive SET_ADDRESS command from the host.
 */
typedef enum usb_state_e
{
    USB_STATE_NONE = 0,
    USB_STATE_RESET,
    USB_STATE_POWERED,
    USB_STATE_DEFAULT,
    USB_STATE_ADDRESS,
    USB_STATE_SUSPENDED
} usb_state_t;

/**
 * @brief USB setup packet data.
 */
typedef union usb_setup_packet_u
{
    struct
    {
        uint8_t request_type;
        uint8_t request;
        uint16_t value;
        uint16_t index;
        uint16_t length;
    };
    struct
    {
        uint8_t request_type;
        uint8_t request;
        uint8_t value_l;
        uint8_t value_h;
        uint8_t index_l;
        uint8_t index_h;
        uint16_t length;
    } detailed;
    
    uint32_t raw_packet_data[2];
} usb_setup_packet_t;


typedef struct usb_init_s
{
    /* public */
    bool vbus_sensing;
    /* private */
} usb_init_t;

typedef struct usb_driver_s
{
    /* public */
    
    /* private */
    usb_state_t state;
    uint32_t device_address;
    usb_setup_packet_t setup_packet;
    uint8_t ep0_tx_buf[64];
    uint32_t ep0_tx_buf_len;
} usb_driver_t;

void usb_init(usb_driver_t *driver, usb_init_t *init);
void usb_irq_handler(void);



#endif /* USB_H */

/*** end of file ***/