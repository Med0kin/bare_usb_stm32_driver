/** @file usb.c
 * 
 * @brief USB peripheral.
 */

#include "usb.h"
#include "usb_internal.h"

#define THIS_FILE__ "usb.c"


static void gpio_init(usb_init_t *init);
static void core_init(usb_init_t *init);
static void device_init(usb_init_t *init);
static void core_soft_reset(void);

static usb_driver_t *p_usb_driver = NULL;


/*##########################################################################*/
/*#                            PUBLIC FUNCTIONS                            #*/
/*##########################################################################*/

/**
 * @brief USB initialization.
 */
void
usb_init(usb_driver_t *p_driver, usb_init_t *init)
{
    p_usb_driver = p_driver;
    gpio_init(init);
    RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
    core_init(init);
    p_usb_driver->state = USB_STATE_NONE;
    device_init(init);
    NVIC_SetPriority(OTG_FS_IRQn, 7);
    NVIC_EnableIRQ(OTG_FS_IRQn);
}

/*##########################################################################*/
/*#                            INTERNAL FUNCTIONS                          #*/
/*##########################################################################*/

usb_driver_t *
usb_get_instance(void)
{
    return p_usb_driver;
}

uint32_t
flush_tx_fifo(void)
{
    while (!(USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL));

    USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_TXFFLSH;
    while(USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH);
    return 1;
}

uint32_t
flush_rx_fifo(void)
{
    while (!(USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL));

    USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_RXFFLSH;
    while(USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH);
    return 1;
}

void
usb_write_fifo(const uint8_t *p_src, uint32_t len)
{
    if (!(USB_EP_IN(0)->DIEPCTL & USB_OTG_DIEPCTL_USBAEP))
    {
        printf("ERR: EP 0 NOT READY!\n");
        return;
    }

    // Check for available space
    uint32_t len_in_words = (len + 3) / 4;
    uint32_t available_space = (USB_EP_IN(0)->DTXFSTS & USB_OTG_DTXFSTS_INEPTFSAV);
    printf("\tAvailable space: %ld\n", available_space);
    if (len_in_words > available_space)
    {
        printf("ERR: Not enough space in TX FIFO!\n");
        return;
    }

    USB_EP_IN(0)->DIEPTSIZ =
        ((1U << USB_OTG_DIEPTSIZ_PKTCNT_Pos) |
         (len << USB_OTG_DIEPTSIZ_XFRSIZ_Pos));

    USB_EP_IN(0)->DIEPCTL &= ~(USB_OTG_DIEPCTL_STALL);
    USB_EP_IN(0)->DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);

    for (uint32_t i = 0; i < (len / 4); i ++)
    {
        USB_OTG_DFIFO(0) = ((uint32_t *)p_src)[i];
    }
    if (len % 4)
    {
        uint32_t remaining = 0;
        memcpy(&remaining, &p_src[len - (len % 4)], len % 4);
        USB_OTG_DFIFO(0) = remaining;
    }

    USB_EP_OUT(0)->DOEPCTL |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
    printf("\tWrote len: %ld\n", len);
}



/*##########################################################################*/
/*#                       STATIC (PRIVATE) FUNCTIONS                       #*/
/*##########################################################################*/

/**
 * @brief USB GPIO initialization.
 *       Initializes the GPIO pins used by the USB peripheral.
 *       For STM32F411RE, the pins are PA11 and PA12.
 */
static void
gpio_init(usb_init_t *init)
{
    // Enable GPIOA clock
    //
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Set PA11 and PA12 for USB functionality
    //
    GPIOA->MODER &= ~(GPIO_MODER_MODER11_0);
    GPIOA->MODER |= GPIO_MODER_MODER11_1;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR11;
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR11);
    GPIOA->AFR[1] |= (GPIO_AFRH_AFSEL11_1 | GPIO_AFRH_AFSEL11_3);

    GPIOA->MODER &= ~(GPIO_MODER_MODER12_0);
    GPIOA->MODER |= GPIO_MODER_MODER12_1;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR12;
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR12);
    GPIOA->AFR[1] |= (GPIO_AFRH_AFSEL12_1 | GPIO_AFRH_AFSEL12_3);

    // Set VBUS sensing pin if enabled
    //
    if (init->vbus_sensing)
    {
        GPIOA->MODER &= ~(GPIO_MODER_MODER9);
        GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR9);
    }

}

/**
 * @brief TEST
 */
static void
core_init(usb_init_t *init)
{
    USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;

    core_soft_reset();

    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN; //TODO CHECK
}

/**
 * @brief TEST
 */
static void
device_init(usb_init_t *init)
{
    USB_OTG_FS->GUSBCFG &= ~(USB_OTG_GUSBCFG_FDMOD |
                             USB_OTG_GUSBCFG_FHMOD);

    USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;

    while(USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_CMOD);

    for (uint32_t i = 0; i < 15U; i++)
    {
        USB_OTG_FS->DIEPTXF[i] = 0U;
    }

    if (init->vbus_sensing)
    {
        USB_OTG_DEVICE->DCTL |= USB_OTG_DCTL_SDIS;
        USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
        USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSBSEN;
        USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSASEN;
    }
    else
    {
        USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_NOVBUSSENS;
        USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBUSBSEN;
    }

    USB_OTG_PCGCCTL = 0U;
    
    USB_OTG_DEVICE->DCFG |= USB_OTG_DCFG_DSPD;

    flush_rx_fifo();
    flush_tx_fifo();

    USB_OTG_DEVICE->DIEPMSK = 0U;
    USB_OTG_DEVICE->DOEPMSK = 0U;
    USB_OTG_DEVICE->DAINTMSK = 0U;

    if (USB_EP_IN(0)->DIEPCTL & USB_OTG_DIEPCTL_EPENA)
    {
        USB_EP_IN(0)->DIEPCTL |= USB_OTG_DIEPCTL_SNAK;
    }
    else
    {
        USB_EP_IN(0)->DIEPCTL = 0U;
    }

    USB_EP_IN(0)->DIEPTSIZ = 0U;
    USB_EP_IN(0)->DIEPINT = 0xFB7FU;

    if (USB_EP_OUT(0)->DOEPCTL & USB_OTG_DOEPCTL_EPENA)
    {
        USB_EP_OUT(0)->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
    }
    else
    {
        USB_EP_OUT(0)->DOEPCTL = 0U;
    }

    USB_EP_OUT(0)->DOEPTSIZ = 0U;
    USB_EP_OUT(0)->DOEPINT = 0xFB7FU;


    USB_OTG_FS->GINTSTS = 0U;
    USB_OTG_FS->GINTSTS = 0xBFFFFFFFU;

    USB_OTG_FS->GINTMSK |= (USB_OTG_GINTMSK_USBRST   |
                            USB_OTG_GINTMSK_ENUMDNEM |
                            USB_OTG_GINTMSK_IEPINT   |
                            USB_OTG_GINTMSK_OEPINT   |
                            USB_OTG_GINTMSK_RXFLVLM  |
                            USB_OTG_GINTMSK_SOFM     |
                            USB_OTG_GINTMSK_MMISM    |
                            USB_OTG_GINTMSK_USBSUSPM |
                            USB_OTG_GINTMSK_WUIM);
    if (init->vbus_sensing)
    {
        USB_OTG_FS->GINTMSK |= (USB_OTG_GINTMSK_SRQIM |
                                USB_OTG_GINTMSK_OTGINT);
    }

    USB_OTG_PCGCCTL &= ~(USB_OTG_PCGCR_STPPCLK | USB_OTG_PCGCR_GATEHCLK);

    USB_OTG_DEVICE->DCTL |= USB_OTG_DCTL_SDIS;

    USB_OTG_FS->GRXFSIZ = USB_GLOBAL_RX_FIFO_SIZE;
    USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = (USB_EP0_TX_FIFO_SIZE
                                      << USB_OTG_DIEPTXF_INEPTXFD_Pos);

    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;

    USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;

    USB_OTG_PCGCCTL &= ~(USB_OTG_PCGCR_STPPCLK | USB_OTG_PCGCR_GATEHCLK);

    USB_OTG_DEVICE->DCTL &= ~(USB_OTG_DCTL_SDIS);
}

/**
 * @brief Reset USB hardware to its default state.
 */
static void
core_soft_reset(void)
{
    while (!(USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL));

    USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;

    while (USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST);
}










/*** end of file ***/