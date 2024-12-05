/*##########################################################################*/
/*#                               INIT                                     #*/
/*##########################################################################*/

/**
 * @brief USB core initialization.
 *        Initialized according to OTG_FS programming model.
 *		  described in the reference manual.
 */
static void
core_init(usb_init_t *init)
{
    // Unmask global interrupt
    // TXFE interrupt indicates that TxFIFO is completely empty
    //
    USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT |
                           USB_OTG_GAHBCFG_TXFELVL;
    USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_PTXFELVL; //TODO: delete it
    
    // Setup GUSBCFG register
    USB_OTG_FS->GUSBCFG &= ~(USB_OTG_GUSBCFG_HNPCAP |
                             USB_OTG_GUSBCFG_SRPCAP |
                             USB_OTG_GUSBCFG_TRDT   |
                             USB_OTG_GUSBCFG_TOCAL);
    USB_OTG_FS->GUSBCFG |= (0x6 << USB_OTG_GUSBCFG_TRDT_Pos);

    // Unmask required interrupts
    //
    USB_OTG_FS->GINTMSK |= (USB_OTG_GINTMSK_OTGINT |
                            USB_OTG_GINTMSK_MMISM);

    
}

/**
 * @brief USB device initialization.
 * 	      Initialized according to OTG_FS programming model
 * 		  described in the reference manual.
 */
static void
device_init(usb_init_t *init)
{
    USB_OTG_DEVICE->DCFG &= !(USB_OTG_DCFG_NZLSOHSK);

    // Clear pending interrupts
    //
    USB_OTG_FS->GINTSTS |= (USB_OTG_GINTSTS_USBRST  |
                            USB_OTG_GINTSTS_ENUMDNE |
                            USB_OTG_GINTSTS_ESUSP   |
                            USB_OTG_GINTSTS_USBSUSP |
                            USB_OTG_GINTSTS_SOF     |

                            USB_OTG_GINTSTS_SRQINT  |
                            USB_OTG_GINTSTS_MMIS    |
                            USB_OTG_GINTSTS_OTGINT);

    // Unmask interrupts
    //
    USB_OTG_FS->GINTMSK |= (USB_OTG_GINTMSK_USBRST   |
                            USB_OTG_GINTMSK_ENUMDNEM |
                            USB_OTG_GINTMSK_ESUSPM   |
                            USB_OTG_GINTMSK_USBSUSPM |
                            USB_OTG_GINTMSK_SOFM     |

                            USB_OTG_GINTMSK_OEPINT   |
                            USB_OTG_GINTMSK_IEPINT   |
                            USB_OTG_GINTMSK_SRQIM    |
                            USB_OTG_GINTMSK_WUIM     |
                            USB_OTG_GINTMSK_RXFLVLM);

    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;
    USB_OTG_DEVICE->DCTL |= USB_OTG_DCTL_SDIS;

    // Enable V_BUS sensing
    //
    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBUSBSEN;
    
    // Force device mode and select FS PHY
    // w
    USB_OTG_FS->GUSBCFG |= (USB_OTG_GUSBCFG_FDMOD |
                            USB_OTG_GUSBCFG_PHYSEL);


    USB_OTG_DEVICE->DCFG |= USB_OTG_DCFG_DSPD;

    // Clear all pending interrupts
    //
    USB_OTG_FS->GINTSTS = 0xFFFFFFFF;
}


/*##########################################################################*/
/*#                              INTERRUPTS                                #*/
/*##########################################################################*/
/**
 * @brief SRQINT interrupt handler.
 * 
 *    Indicates that there is an active host
 *    In order to make sure the session is valid, it's also required
 *    to check if B-session valid bit is set in the OTG_FS_GOTGCTL register.
 *    It might happen that this interrupt is triggered by a noise on the USB lines.
 *   
 */
static void
srqint_handler(void)
{
    USB_OTG_FS->GINTSTS |= USB_OTG_GINTSTS_SRQINT;
    printf("SRQINT\n");

    if (!(USB_OTG_FS->GOTGCTL & USB_OTG_GOTGCTL_BSVLD))
    {
        return;
    }

    ENSURE(usb_driver->state == USB_STATE_NONE);

    // Soft connect
    //
    USB_OTG_DEVICE->DCTL &= ~(USB_OTG_DCTL_SDIS);

    usb_driver->state = USB_STATE_POWERED;
}

/**
 * @brief USBRST interrupt handler.
 */
static void
usbrst_handler(void)
{
    USB_OTG_FS->GINTSTS |= USB_OTG_GINTSTS_USBRST;
    printf("USBRST\n");

    if (!(usb_driver->state == USB_STATE_POWERED)) return; //TODO

    endpoint_reset_init();

    usb_driver->state = USB_STATE_RESET;
}

/**
 * @brief ENUMDNE interrupt handler.
 */
static void
enumdne_handler(void)
{
    USB_OTG_FS->GINTSTS |= USB_OTG_GINTSTS_ENUMDNE;
    printf("ENUMDNE\n");
    
    if (!(usb_driver->state == USB_STATE_RESET)) return; //TODO

    endpoint_enum_init();
        
    usb_driver->state = USB_STATE_DEFAULT;

    // TODO: wait for addres and read device address
}

/*##########################################################################*/
/*#                                                                        #*/
/*##########################################################################*/

/**
 * @brief Endpoint initialization on USB reset.
 */
static void
endpoint_reset_init(void)
{
    // Unmask interrupts
    //
    USB_OTG_DEVICE->DAINTMSK |= 0x01 << USB_OTG_DAINTMSK_IEPM_Pos;
    USB_OTG_DEVICE->DAINTMSK |= 0x03 << USB_OTG_DAINTMSK_OEPM_Pos;

    USB_OTG_DEVICE->DOEPMSK |= (USB_OTG_DOEPMSK_STUPM |
                                USB_OTG_DOEPMSK_XFRCM);

    USB_OTG_DEVICE->DIEPMSK |= (USB_OTG_DIEPMSK_XFRCM |
                                USB_OTG_DIEPMSK_TOM);

    // Flush all FIFOs
    //
    flush_tx_fifo();
    flush_rx_fifo();

    // Set address to 0
    //
    USB_OTG_DEVICE->DCFG &= ~USB_OTG_DCFG_DAD;

    // Set Rx FIFO size
    // this must be equal to 1 max packet size of control endpoint 0 + 2 words
    // (for the status of the control OUT data packet) + 10 words (for setup packets).
    // for 64 bytes max packet size (16 words) 16 + 2 + 10 = 28
    //
    USB_OTG_FS->GRXFSIZ = 36;

    // Set Tx FIFO size for EP0
    //
    USB_OTG_FS->DIEPTXF0_HNPTXFSIZ |= (16 << USB_OTG_DIEPTXF_INEPTXFD_Pos);

    USB_EP_OUT(0)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_STUPCNT |
                                (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos) |
                                (USB_EP0_RX_FIFO_SIZE << USB_OTG_DOEPTSIZ_XFRSIZ_Pos));

    // Clear NAK and enable EP0
    //
    USB_EP_OUT(0)->DOEPCTL |= (USB_OTG_DOEPCTL_CNAK |
                               USB_OTG_DOEPCTL_EPENA);

}

/**
 * @brief Endpoint initialization on enumeration completion.
 */
static void
endpoint_enum_init(void)
{
    // Enumeration speed
    //
    uint8_t enum_speed = (USB_OTG_DEVICE->DSTS & USB_OTG_DSTS_ENUMSPD) >> USB_OTG_DSTS_ENUMSPD_Pos;

    // Set the maximum packet size for EP0
    //
    USB_EP_IN(0)->DIEPCTL &= ~(0x03);
}
