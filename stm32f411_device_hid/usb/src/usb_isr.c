/**
 * @file usb_isr.c
 */

#include "usb.h"
#include "usb_internal.h"
#include "usb_desc.h"

#define THIS_FILE__ "usb_isr.c"

static void mmis_handler(usb_driver_t *p_driver);
static void usbrst_handler(usb_driver_t *p_driver);
static void enumdne_handler(usb_driver_t *p_driver);
static void rxflvl_handler(usb_driver_t *p_driver);

static void oepint_handler(usb_driver_t *p_driver);
static void oepint_stup_handler(usb_driver_t *p_driver);
static void oepint_stup_get_descriptor_handler(usb_driver_t *p_driver);

static void iepint_handler(usb_driver_t *p_driver);

static const void (*gintsts_handlers[])(usb_driver_t *) = {
    NULL,               /* CMOD */
    mmis_handler,       /* MMIS */
    NULL,               /* OTGINT */
    NULL,               /* SOF */
    rxflvl_handler,     /* RXFLVL */
    NULL,               /* NPTXFE */
    NULL,               /* GINNAKEFF */
    NULL,               /* GOUTNAKEFF */
    NULL, NULL,
    NULL,               /* ESUSP */
    NULL,               /* USBSUSP */
    usbrst_handler,     /* USBRST */
    enumdne_handler,    /* ENUMDNE */
    NULL,               /* ISOOUTDROP */
    NULL,               /* EOPF */
    NULL, NULL,
    iepint_handler,     /* IEPINT */
    oepint_handler,     /* OEPINT */
    NULL,               /* IISOIXFR */
    NULL,               /* IPXFR_INCOMPISOOUT */
    NULL, NULL,
    NULL,               /* HPRTINT */
    NULL,               /* HCINT */
    NULL,               /* PTXFE */
    NULL,
    NULL,               /* CIDSCHG */
    NULL,               /* DISCINT */
    NULL,               /* SRQINT */
    NULL                /* WKUINT */
};
#define GINTSTS_HANDLERS_SIZE (sizeof(gintsts_handlers) / sizeof(gintsts_handlers[0]))

/**
 * @brief USB interrupt handler.
 *        Detects the source of the interrupt and calls the appropriate handler.
 */
void
usb_irq_handler(void)
{
    usb_driver_t *p_driver = usb_get_instance();

    if (p_driver == NULL)
    {
        return;
    }
    
    uint32_t gintsts_reg = USB_OTG_FS->GINTSTS;

    for (size_t interrupt = 1; interrupt < GINTSTS_HANDLERS_SIZE; interrupt++)
    {
        if (gintsts_reg & (1 << interrupt))
        {
            if (gintsts_handlers[interrupt] != NULL)
            {
                gintsts_handlers[interrupt](p_driver);
            }
            USB_OTG_FS->GINTSTS |= (1 << interrupt);
        }
    }

}

/*##########################################################################*/
/*#                       GINTSTS INTERRUPT HANDLERS                       #*/
/*##########################################################################*/

/**
 * @brief Mode mismatch interrupt handler.
 */
static void
mmis_handler(usb_driver_t *p_driver)
{
    printf("MMIS\n");
    ASSERT(0);
}

/**
 * @brief USB reset interrupt handler.
 */
static void
usbrst_handler(usb_driver_t *p_driver)
{
    printf("USBRST\n");
    USB_OTG_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;
    flush_tx_fifo();
    USB_EP_IN(0)->DIEPINT = 0xFB7FU;
    USB_EP_IN(0)->DIEPCTL &= ~(USB_OTG_DIEPCTL_STALL);
    USB_EP_OUT(0)->DOEPINT = 0xFB7FU;
    USB_EP_OUT(0)->DOEPCTL &= ~(USB_OTG_DOEPCTL_STALL);
    USB_EP_OUT(0)->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;

    USB_OTG_DEVICE->DAINTMSK |= 0x10001U;

    USB_OTG_DEVICE->DOEPMSK |= (USB_OTG_DOEPMSK_STUPM    |
                                USB_OTG_DOEPMSK_XFRCM    |
                                USB_OTG_DOEPMSK_EPDM     |
                                USB_OTG_DOEPMSK_OTEPSPRM |
                                USB_OTG_DOEPMSK_NAKM);

    USB_OTG_DEVICE->DIEPMSK |= (USB_OTG_DIEPMSK_TOM   |
                                USB_OTG_DIEPMSK_XFRCM |
                                USB_OTG_DIEPMSK_EPDM);

    USB_OTG_DEVICE->DCFG &= ~(USB_OTG_DCFG_DAD);

    USB_EP_OUT(0)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_STUPCNT |
                                (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos) |
                                ((USB_EP0_RX_FIFO_SIZE) << USB_OTG_DOEPTSIZ_XFRSIZ_Pos));
}

/**
 * @brief ENUMDNE interrupt handler.
 */
static void
enumdne_handler(usb_driver_t *p_driver)
{
    printf("ENUMDNE\n");
    USB_EP_IN(0)->DIEPCTL &= ~(USB_OTG_DIEPCTL_MPSIZ);
    USB_EP_OUT(0)->DOEPCTL &= ~(USB_OTG_DOEPCTL_MPSIZ);
    
    USB_OTG_DEVICE->DCTL |= USB_OTG_DCTL_CGINAK;
    USB_OTG_FS->GUSBCFG |= (0x6 << USB_OTG_GUSBCFG_TRDT_Pos);

    USB_OTG_DEVICE->DAINTMSK = 0x10001U;

    USB_EP_IN(0)->DIEPCTL |= USB_OTG_DIEPCTL_USBAEP;
    USB_EP_OUT(0)->DOEPCTL |= USB_OTG_DOEPCTL_USBAEP;

    // Clear pending DIEPINT interrupts
    USB_EP_IN(0)->DIEPINT = 0xFB7FU;
}

/**
 * @brief RXFLVL interrupt handler.
 *          TODO: think of adding tx fifo setting here and not in oepint_handler
 */
static void
rxflvl_handler(usb_driver_t *p_driver)
{
    printf("RXFLVL\n");
    USB_OTG_FS->GINTMSK &= ~USB_OTG_GINTMSK_RXFLVLM;

    uint32_t grxstsp_val = USB_OTG_FS->GRXSTSP;

    enum usb_rx_status_e status = (grxstsp_val & USB_OTG_GRXSTSP_PKTSTS)
                                >> USB_OTG_GRXSTSP_PKTSTS_Pos;
    uint32_t byte_count = (grxstsp_val & USB_OTG_GRXSTSP_BCNT)
                            >> USB_OTG_GRXSTSP_BCNT_Pos;
    uint32_t ep_num = (grxstsp_val & USB_OTG_GRXSTSP_EPNUM)
                            >> USB_OTG_GRXSTSP_EPNUM_Pos;
    uint32_t data_pid = (grxstsp_val & USB_OTG_GRXSTSP_DPID)
                        >> USB_OTG_GRXSTSP_DPID_Pos;
    
    printf("\tstat: %d, b_cnt: %ld, ep: %ld, pid: %ld\n", status, byte_count, ep_num, data_pid);

    
    uint32_t word_count = (byte_count + 3) / 4;
    uint32_t *p_data_dst = NULL;
    switch(status)
    {
        case USB_RX_STATUS_NAK:
            break;
        case USB_RX_STATUS_DATA_UPDT:
            break;
        case USB_RX_STATUS_XFER_COMP:
            break;
        case USB_RX_STATUS_SETUP_COMP:
            break;
        case USB_RX_STATUS_SETUP_UPDT:
            ENSURE((ep_num == 0) && (byte_count == 8));
            p_data_dst = (uint32_t *)(&p_driver->setup_packet.raw_packet_data);
            for (uint32_t word = 0; word < word_count; word++)
            {
                *p_data_dst = USB_OTG_DFIFO(ep_num);
                p_data_dst++;
            }
            //flush_rx_fifo();
            break;
        default:
            break;
    }
    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;
}

/*##########################################################################*/
/*#                        OEPINT INTERRUPT HANDLERS                       #*/
/*##########################################################################*/

/**
 * @brief OEPINT interrupt handler.
 */
static void
oepint_handler(usb_driver_t *p_driver)
{
    printf("OEPINT\n");
    uint32_t daint_reg = USB_OTG_DEVICE->DAINT;
    uint32_t ep_num_one_hot = (daint_reg & USB_OTG_DAINT_OEPINT)
                               >> (USB_OTG_DAINT_OEPINT_Pos);
    REQUIRE(ep_num_one_hot != 0);
    uint32_t ep_num = __builtin_ctz(ep_num_one_hot);
    uint32_t doepint_reg = USB_EP_OUT(ep_num)->DOEPINT;

    if (doepint_reg & USB_OTG_DOEPINT_XFRC)
    {
        printf("\tXFRC out%ld\n", ep_num);
        USB_EP_OUT(ep_num)->DOEPINT |= USB_OTG_DOEPINT_XFRC;
    }
    if (doepint_reg & USB_OTG_DOEPINT_EPDISD)
    {
        printf("\tEPDISD out%ld\n", ep_num);
        USB_EP_OUT(ep_num)->DOEPINT |= USB_OTG_DOEPINT_EPDISD;
    }
    if (doepint_reg & USB_OTG_DOEPINT_STUP)
    {
        ENSURE(ep_num == 0);
        oepint_stup_handler(p_driver);
        USB_EP_OUT(ep_num)->DOEPINT |= USB_OTG_DOEPINT_STUP;
    }
    if (doepint_reg & USB_OTG_DOEPINT_OTEPDIS)
    {
        printf("\tOTEPDIS out%ld\n", ep_num);
        USB_EP_OUT(ep_num)->DOEPINT |= USB_OTG_DOEPINT_OTEPDIS;
    }
    if (doepint_reg & USB_OTG_DOEPINT_NAK)
    {
        printf("\tNAK out%ld\n", ep_num);
        USB_EP_OUT(ep_num)->DOEPINT |= USB_OTG_DOEPINT_NAK;
    }
}

/**
 * @brief OEPINT interrupt handler for SETUP packets.
 */
static void
oepint_stup_handler(usb_driver_t *p_driver)
{
    uint32_t addr = 0;
    enum usb_request_e request = (enum usb_request_e)p_driver->setup_packet.request;
    printf("\tstup_req:%d\n", request);
    switch(request)
    {
        case USB_BREQUEST_GET_STATUS:
            break;
        case USB_BREQUEST_CLEAR_FEATURE:
            break;
        case USB_BREQUEST_SET_FEATURE:
            break;
        case USB_BREQUEST_SET_ADDRESS:
            addr = p_driver->setup_packet.value;
            USB_OTG_DEVICE->DCFG |= (addr << USB_OTG_DCFG_DAD_Pos);
            usb_write_fifo(NULL, 0);
            break;
        case USB_BREQUEST_GET_DESCRIPTOR:
            oepint_stup_get_descriptor_handler(p_driver);
            break;
        case USB_BREQUEST_SET_DESCRIPTOR:
            break;
        case USB_BREQUEST_GET_CONFIGURATION:
            break;
        case USB_BREQUEST_SET_CONFIGURATION:
            break;
        case USB_BREQUEST_GET_INTERFACE:
            break;
        case USB_BREQUEST_SET_INTERFACE:
            break;
        case USB_BREQUEST_SYNCH_FRAME:
            break;
        default:
            printf("\tUnknown request or not supported\n");
            break;
    }
}

static void
oepint_stup_get_descriptor_handler(usb_driver_t *p_driver)
{
    uint16_t value = p_driver->setup_packet.value;
    enum usb_descriptor_value_e desc_value_type = (enum usb_descriptor_value_e)(value >> 8);
    uint32_t len_requested = p_driver->setup_packet.length;
    const uint8_t *p_descriptor_requested = NULL;
    uint32_t len = 0;

    printf("\tdesc_type:%d\n", desc_value_type);

    switch(desc_value_type)
    {
        case USB_DESCRIPTOR_DEVICE:
            p_descriptor_requested = (const uint8_t *)device_descriptor;
            len = sizeof(device_descriptor);
            break;
        case USB_DESCRIPTOR_CONFIGURATION:
            p_descriptor_requested = (const uint8_t *)configuraiton_descritor;
            len = sizeof(configuraiton_descritor);
            break;
        case USB_DESCRIPTOR_STRING:
            break;
        case USB_DESCRIPTOR_INTERFACE:
            break;
        case USB_DESCRIPTOR_ENDPOINT:
            break;
        case USB_DESCRIPTOR_DEVICE_QUALIFIER:
            break;
        case USB_DESCRIPTOR_OTHER_SPEED_CONFIGURATION:
            break;
        case USB_DESCRIPTOR_INTERFACE_POWER:
            break;
        case USB_DESCRIPTOR_OTG:
            break;
        default:
            ASSERT(0);
            return;
    }

    if (len > len_requested)
    {
        len = len_requested;
    }
    if (p_descriptor_requested != NULL && len > 0)
    {
        usb_write_fifo(p_descriptor_requested, len);
    }
    else
    {
        //TODO when desc not support it should STALL n not just ignore
        printf("\tDescriptor not fount or len is zero\n");
    }
}

/*##########################################################################*/
/*#                       IEPINT INTERRUPT HANDLERS                        #*/
/*##########################################################################*/

/**
 * @brief IEPINT interrupt handler.
 */
static void
iepint_handler(usb_driver_t *p_driver)
{
    printf("IEPINT\n");
    uint32_t daint_reg = USB_OTG_DEVICE->DAINT;
    uint32_t ep_num_one_hot = (daint_reg & USB_OTG_DAINT_IEPINT)
                               >> (USB_OTG_DAINT_IEPINT_Pos);
    uint32_t ep_num = __builtin_ctz(ep_num_one_hot);

    uint32_t iepint_reg = USB_EP_IN(ep_num)->DIEPINT;

    if (iepint_reg & USB_OTG_DIEPINT_XFRC)
    {
        printf("XFRC %ld\n", ep_num);
        // Prepare for next reception
        USB_EP_OUT(ep_num)->DOEPTSIZ |= (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos);
        USB_EP_IN(ep_num)->DIEPINT |= USB_OTG_DIEPINT_XFRC;
    }
    if (iepint_reg & USB_OTG_DIEPINT_EPDISD)
    {
        printf("EPDISD %ld\n", ep_num);
        USB_EP_IN(ep_num)->DIEPINT |= USB_OTG_DIEPINT_EPDISD;
    }
    if (iepint_reg & USB_OTG_DIEPINT_TOC)
    {
        printf("TOC %ld\n", ep_num);
        USB_EP_IN(ep_num)->DIEPINT |= USB_OTG_DIEPINT_TOC;
    }
    if (iepint_reg & USB_OTG_DIEPINT_ITTXFE)
    {
        printf("ITTXFE %ld\n", ep_num);
        USB_EP_IN(ep_num)->DIEPINT |= USB_OTG_DIEPINT_ITTXFE;
    }
    if (iepint_reg & USB_OTG_DIEPINT_INEPNE)
    {
        printf("INEPNE %ld\n", ep_num);
        USB_EP_IN(ep_num)->DIEPINT |= USB_OTG_DIEPINT_INEPNE;
    }
    if (iepint_reg & USB_OTG_DIEPINT_TXFE)
    {
        printf("TXFE %ld\n", ep_num);
        USB_EP_IN(ep_num)->DIEPINT |= USB_OTG_DIEPINT_TXFE;
    }
    if (iepint_reg & USB_OTG_DIEPINT_PKTDRPSTS)
    {
        printf("PKTDRPSTS %ld\n", ep_num);
        USB_EP_IN(ep_num)->DIEPINT |= USB_OTG_DIEPINT_PKTDRPSTS;
    }
    if (iepint_reg & USB_OTG_DIEPINT_NAK)
    {
        printf("NAK %ld\n", ep_num);
        USB_EP_IN(ep_num)->DIEPINT |= USB_OTG_DIEPINT_NAK;
    }
}

/* end of file */
