#include "board.h"
#include "bme280_spi.h"
#include "bme280.h"
#include "app_logic.h"
#include "commands.h"
#include "app_usbd_cfg.h"
#include "cdc_vcom.h"
#include "string.h"
#include <cr_section_macros.h>

struct bme280_dev *dev = NULL;
static USBD_HANDLE_T g_hUsb;
static uint8_t g_rxBuff[64];
const  USBD_API_T *g_pUsbApi;

/**
 * @brief	Handle interrupt from USB0
 * @return	Nothing
 */
void USB_IRQHandler(void)
{
	uint32_t *addr = (uint32_t *) LPC_USB->EPLISTSTART;

	/*	WORKAROUND for artf32289 ROM driver BUG:
	    As part of USB specification the device should respond
	    with STALL condition for any unsupported setup packet. The host will send
	    new setup packet/request on seeing STALL condition for EP0 instead of sending
	    a clear STALL request. Current driver in ROM doesn't clear the STALL
	    condition on new setup packet which should be fixed.
	 */
	if ( LPC_USB->DEVCMDSTAT & _BIT(8) ) {	/* if setup packet is received */
		addr[0] &= ~(_BIT(29));	/* clear EP0_OUT stall */
		addr[2] &= ~(_BIT(29));	/* clear EP0_IN stall */
	}
	USBD_API->hw->ISR(g_hUsb);
}

/* Find the address of interface descriptor for given class type. */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
	USB_COMMON_DESCRIPTOR *pD;
	USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
	uint32_t next_desc_adr;

	pD = (USB_COMMON_DESCRIPTOR *) pDesc;
	next_desc_adr = (uint32_t) pDesc;

	while (pD->bLength) {
		/* is it interface descriptor */
		if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

			pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
			/* did we find the right interface descriptor */
			if (pIntfDesc->bInterfaceClass == intfClass) {
				break;
			}
		}
		pIntfDesc = 0;
		next_desc_adr = (uint32_t) pD + pD->bLength;
		pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
	}

	return pIntfDesc;
}

int main(void) {

	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;
	uint32_t prompt = 0, rdCnt = 0;

	SystemCoreClockUpdate();
	/* Initialize board and chip */
	Board_Init();
	Board_LED_Set(0, 0);

	/* enable clocks and pinmux */
	Chip_USB_Init();

	/* initialize USBD ROM API pointer. */
	g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

	/* initialize call back structures */
	memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB0_BASE;
	/*	WORKAROUND for artf44835 ROM driver BUG:
	    Code clearing STALL bits in endpoint reset routine corrupts memory area
	    next to the endpoint control data. For example When EP0, EP1_IN, EP1_OUT,
	    EP2_IN are used we need to specify 3 here. But as a workaround for this
	    issue specify 4. So that extra EPs control structure acts as padding buffer
	    to avoid data corruption. Corruption of padding memory doesn’t affect the
	    stack/program behaviour.
	 */
	usb_param.max_num_ep = 3 + 1;
	usb_param.mem_base = USB_STACK_MEM_BASE;
	usb_param.mem_size = USB_STACK_MEM_SIZE;

	/* Set the USB descriptors */
	desc.device_desc = (uint8_t *) &USB_DeviceDescriptor[0];
	desc.string_desc = (uint8_t *) &USB_StringDescriptor[0];
	/* Note, to pass USBCV test full-speed only devices should have both
	   descriptor arrays point to same location and device_qualifier set to 0.
	 */
	desc.high_speed_desc = (uint8_t *) &USB_FsConfigDescriptor[0];
	desc.full_speed_desc = (uint8_t *) &USB_FsConfigDescriptor[0];
	desc.device_qualifier = 0;

	/* USB Initialization */
	ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
	if (ret == LPC_OK) {

		/*	WORKAROUND for artf32219 ROM driver BUG:
		    The mem_base parameter part of USB_param structure returned
		    by Init() routine is not accurate causing memory allocation issues for
		    further components.
		 */
		usb_param.mem_base = USB_STACK_MEM_BASE + (USB_STACK_MEM_SIZE - usb_param.mem_size);

		/* Init VCOM interface */
		ret = vcom_init(g_hUsb, &desc, &usb_param);
		if (ret == LPC_OK) {
			/*  enable USB interrupts */
			NVIC_EnableIRQ(USB0_IRQn);
			/* now connect */
			USBD_API->hw->Connect(g_hUsb, 1);
		}

	}

	/* 1s interrupt */
	SysTick_Config(SystemCoreClock / 1000);

    /* SPI init */
    bme280_spi conf = { LPC_SSP1, bme280_default_ssel_init() };
    bme280_spi_init(&conf);

    /* BME280 init */
    struct bme280_dev dev_conf;
    dev = &dev_conf;
    bme280_get_default_conf();

    /* Wait until bme280 sensor respond */
    int8_t rslt = !BME280_OK;
    while (rslt != BME280_OK){
    	rslt = bme280_init(dev);
    }

    /* Set all settings needed */
    bme280_prepare_broadcast();

    /* Send data frames */
    while (1) {
		/* Check if host has connected and opened the VCOM port */
		if ((vcom_connected() != 0) && (prompt == 0)) {
			prompt = 1;
			Board_LED_Set(0, 1);
		}
		/* If VCOM port is opened echo whatever we receive back to host. */
		if (prompt) {
			rdCnt = vcom_bread(&g_rxBuff[0], 64);
			if (rdCnt) {
				invoke_cmd((char *) g_rxBuff);
			}
		}
    }

    return 0 ;
}
