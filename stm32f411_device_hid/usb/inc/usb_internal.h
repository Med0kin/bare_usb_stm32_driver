/**
 * @file usb_private.h
 * 
 * @brief Consists of internal USB functions and structures.
 */

#ifndef USB_PRIVATE_H
#define USB_PRIVATE_H

#include "usb.h"

#define USB_GLOBAL_RX_FIFO_SIZE (128U)
#define USB_EP0_RX_FIFO_SIZE    (64U)
#define USB_EP0_TX_FIFO_SIZE    (64U)

#define USB_OTG_DEVICE           ((USB_OTG_DeviceTypeDef *) (USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE))
#define USB_EP_OUT(ep_num) 		 ((USB_OTG_OUTEndpointTypeDef *) ((USB_OTG_FS_PERIPH_BASE +  USB_OTG_OUT_ENDPOINT_BASE) + ((ep_num) * USB_OTG_EP_REG_SIZE)))
#define USB_EP_IN(ep_num)    	 ((USB_OTG_INEndpointTypeDef *)	((USB_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE) + ((ep_num) * USB_OTG_EP_REG_SIZE)))
#define USB_OTG_DFIFO(ep_num)    (*(volatile uint32_t *)(USB_OTG_FS_PERIPH_BASE  + USB_OTG_FIFO_BASE + ((ep_num) * USB_OTG_FIFO_SIZE)))
#define USB_OTG_PCGCCTL          (*(volatile uint32_t *)( USB_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE))


/**
 * @brief RX status.
 *        Used to determine the status of the received data
 *        in the RX FIFO during the RXFLVL interrupt.
 */
enum usb_rx_status_e
{
    USB_RX_STATUS_NAK        = 1,
    USB_RX_STATUS_DATA_UPDT  = 2,
    USB_RX_STATUS_XFER_COMP  = 3,
    USB_RX_STATUS_SETUP_COMP = 4,
    USB_RX_STATUS_SETUP_UPDT = 6
};

/**
 * @brief Request values.
 */
enum usb_request_e
{
    USB_BREQUEST_GET_STATUS = 0,
    USB_BREQUEST_CLEAR_FEATURE = 1,
    USB_BREQUEST_SET_FEATURE = 3,
    USB_BREQUEST_SET_ADDRESS = 5,
    USB_BREQUEST_GET_DESCRIPTOR = 6,
    USB_BREQUEST_SET_DESCRIPTOR = 7,
    USB_BREQUEST_GET_CONFIGURATION = 8,
    USB_BREQUEST_SET_CONFIGURATION = 9,
    USB_BREQUEST_GET_INTERFACE = 10,
    USB_BREQUEST_SET_INTERFACE = 11,
    USB_BREQUEST_SYNCH_FRAME = 12
};

/**
 * @brief Value in descriptor request.
 */
enum usb_descriptor_value_e
{
    USB_DESCRIPTOR_DEVICE = 1,
    USB_DESCRIPTOR_CONFIGURATION = 2,
    USB_DESCRIPTOR_STRING = 3,
    USB_DESCRIPTOR_INTERFACE = 4,
    USB_DESCRIPTOR_ENDPOINT = 5,
    USB_DESCRIPTOR_DEVICE_QUALIFIER = 6,
    USB_DESCRIPTOR_OTHER_SPEED_CONFIGURATION = 7,
    USB_DESCRIPTOR_INTERFACE_POWER = 8,
    USB_DESCRIPTOR_OTG = 9,
    USB_DESCRIPTOR_REPORT = 0x22
};

uint32_t flush_tx_fifo(void);
uint32_t flush_rx_fifo(void);
void usb_write_fifo(const uint8_t *src, size_t len);
usb_driver_t *usb_get_instance(void);

#endif /* USB_PRIVATE_H */

/*** end of file ***/