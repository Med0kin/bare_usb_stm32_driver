/**
 * @file usb_descriptor.h
 */

#ifndef USB_DESC_H
#define USB_DESC_H

#include <stdint.h>

#include "usb.h"
#include "usb_internal.h"

#define MAJOR_VER 0x01
#define MINOR_VER 0x00

// String descriptor indexes
//
#define USB_DESC_STRING_MANUFACTURER 0x00
#define USB_DESC_STRING_PRODUCT      0x01

/**
 * @brief Device descriptor
 *        The HID class is specified in the interface descriptor
 *        for more flexibility.
 */
const uint8_t device_descriptor[18] = {
    18,                             /* bLength */
    0x01,                           /* bDescriptorType:     Device Descriptor*/
    0x00, 0x02,                     /* bcdUSB:              USB 2.0 */
    0x00,                           /* bDeviceClass:        No class at device level */
    0x00,                           /* bDeviceSubClass      No subclass */
    0x00,                           /* bDeviceProtocol      No protocol */
    USB_EP0_RX_FIFO_SIZE,           /* bMaxPacketSize0*/
    0x11, 0x11,                     /* idVendor             Prototype vendor id*/
    0x11, 0x11,                     /* idProduct            Prototype vendor id*/
    MAJOR_VER, MAJOR_VER,           /* bcdDevice */
    USB_DESC_STRING_MANUFACTURER,   /* iManufacturer */
    USB_DESC_STRING_PRODUCT,        /* iProduct */
    0x00,                           /* iSerialNumber        No serial number string*/
    0x01                            /* bNumConfigurations */
};

const uint8_t configuraiton_descritor[25] = {
    9,                              /* bLength */
    0x02,                           /* dDescriptorType:     Configuration Descriptor*/
    (9+9+7), 0x00,                  /* wTotalLength:        Total number of bytes in this and following desscriptors*/
    0x01,                           /* bNumInterfaces:      Number of interfaces supported by this configuration*/
    0x01,                           /* bConfigurationValue: Used by SET CONFIGURATION to select this desc*/
    0x00,                           /* iConfiguration:      Index of string descriptor describing configuration*/
    0b11000000,                     /* bmAttributes:        D6: Self-powered, D5: Remote Wakeup*/
    0x01,                           /* bMaxPower:           in units of 2mA*/
    9,                              /* bLength */
    0x04,                           /* dDescriptorType:     Interface Descriptor*/
    0x01,                           /* bInterfaceNumber:    ID number*/
    0x01,                           /* bAletrnateSetting:   Used to select alternate setting*/
    0x01,                           /* bNumEndpoints:       Number of endpoints used by this interface*/
    0x03,                           /* bInterfaceClass      Human Interface Device*/
    0x01,                           /* bInterfaceSubClass   Support boot protocol*/
    0x01,                           /* bInterfaceProtocol   Keyboard*/
    0x00,                           /* iInterface*/
    7,                              /* bLength */
    0x05,                           /* dDescriptorType:     Endpoint Descriptor*/
    0b10000001,                     /* bEndpointAddress:    D3-D0: endpoint number, D7: IN direciton*/
    0x03,                           /* bmAttribures:        Interrupt*/
    0x08, 0x00,                     /* wMaxPacketSize       8bytes*/
    0x0A                            /* bInterval:           10ms*/
};

/*##########################################################################*/
/*#                           STRING DESCRIPTORS                           #*/
/*##########################################################################*/

const uint8_t string_descriptor_language[] = {
    4,                              /* bLength */
    0x03,                           /* bDescriptorType:     String Descriptor */
    0x09, 0x04                      /* Language ID:         English (United States) */
};

/**
 * @brief String descriptor for manufacturer.
 */
const uint8_t string_descriptor_manufacturer[] = {
    16,                             /* bLength */
    0x03,                           /* bDescriptorType:     String Descriptor */
    'M', 0x00,                      /* Manufacturer */
    'E', 0x00,
    'D', 0x00,
    '0', 0x00,
    'K', 0x00,
    'I', 0x00,
    'N', 0x00
};

/**
 * @brief String descriptor for product.
 */
const uint8_t string_descriptor_product[] = {
    18,                             /* bLength */
    0x03,                           /* bDescriptorType:     String Descriptor */
    'K', 0x00,                      /* Product */
    'e', 0x00,
    'y', 0x00,
    'b', 0x00,
    'o', 0x00,
    'a', 0x00,
    'r', 0x00,
    'd', 0x00
};

/**
 * @brief String descriptor table.
 */
const uint8_t *string_descriptor[] = {
    string_descriptor_language,
    string_descriptor_manufacturer,
    string_descriptor_product
};





#endif /* USB_DESC_H */

/*** end of file ***/