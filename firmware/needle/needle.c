/*
 * Modified from https://github.com/cnlohr/ch32fun/blob/master/examples_ch5xx/ble_beacon/ble_beacon.c
 * License for original source located at https://github.com/cnlohr/ch32fun/blob/master/LICENSE
 */


#include "ch32fun.h"
#include "ch5xx_lowpower.h"
#include "iSLER.h"
#include "fsusb.h"
#include <stdio.h>

#define ACCESS_ADDRESS 0x8E89BED6 // the "BED6" address for BLE advertisements

#define SLEEPTIME_MS 1'000

#define XT32M_LOAD_20PF 0b111

#define USB_SERIAL_DEBUG 1 // 1: usb serial, no deep sleep. 0: deep sleep, no serial.
#define BOOT_WINDOW_MS 3'000
#define BOOT_COMMAND "BOOT"

#define PG_PIN 10
#define RB_SLP_GPIO_EDGE_MODE 0x04 // missing from ch5xxhw.h

static volatile uint8_t usb_plugged;

// The advertisement to be sent. The MAC address should be in the first 6 bytes in reversed byte order,
// after that any BLE flag can be used.
const uint8_t adv_data[] = {
		0x02, 0x0f, // header for LL: PDU + frame length
		0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // MAC (reversed)
		0x08, 0x09, 'c', 'h', '3', '2', 'f', 'u', 'n'}; // 0x09: "Complete Local Name"


ISLER_BUF_ATTR uint8_t adv[256];

uint8_t adv_channels[] = {37,38,39};

void HandleUSBInput(int numbytes, uint8_t *data) {
	if(numbytes >= sizeof(BOOT_COMMAND) - 1 && memcmp(data, BOOT_COMMAND, sizeof(BOOT_COMMAND) - 1) == 0) {
		USBFSReset();
		__disable_irq(); // match reset state so no stale IRQ vectors into flash while the bootloader erases it
		jump_isprom();
	}
}

__INTERRUPT
void RTC_IRQHandler(void) {
	R8_RTC_FLAG_CTRL = RB_RTC_TRIG_CLR;
}

__INTERRUPT
void GPIOA_IRQHandler(void) {
	R16_PA_INT_IF = 1 << PG_PIN;
	usb_plugged = 1;
}

void allPinPullUp(void)
{
	R32_PA_DIR = 0;
	R32_PA_PD_DRV = 0;
	R32_PA_PU = P_All;
#if PB
	R32_PB_DIR = 0;
	R32_PB_PD_DRV = 0;
	R32_PB_PU = P_All;
#endif
}

void idle_ms(int ms){
    LowPowerIdle( MS_TO_RTC(ms) );
}

void usbBootWindow(void) {
	R32_PA_PU &= ~0b11;
	USBFSSetup();
	for(int ms = 0; ms < BOOT_WINDOW_MS; ms += 10) {
		poll_input();
		Delay_Ms(10);
	}
	USBFSReset();
	allPinPullUp();
}


void pgWakeInit(void) {
	R16_PA_INT_MODE |= 1 << PG_PIN;
	R16_PA_INT_IF = 1 << PG_PIN;
	R16_PA_INT_EN |= 1 << PG_PIN;
	SYS_SAFE_ACCESS(
		R8_SLP_WAKE_CTRL |= RB_SLP_GPIO_WAKE | RB_GPIO_WAKE_MODE | RB_SLP_GPIO_EDGE_MODE | RB_WAKE_EV_MODE;
	);
	NVIC_EnableIRQ(GPIOA_IRQn);
}

int main() {
	SystemInit();
	SYS_SAFE_ACCESS(
		R8_XT32M_TUNE = (R8_XT32M_TUNE & ~RB_XT32M_C_LOAD) | (XT32M_LOAD_20PF << 4);
	);

	DCDCEnable(); // Enable the internal DCDC
	LSIEnable(); // Disable LSE, enable LSI
	RTC_init(); // Set the RTC counter to 0
	SleepInit(); // Enable wakeup from sleep by RTC, and enable RTC IRQ

	allPinPullUp(); // this reduces sleep from ~70uA to 1uA

	funGpioInitAll();

#if USB_SERIAL_DEBUG
	R32_PA_PU &= ~0b11;
	USBFSSetup();
#else
	pgWakeInit();
	usbBootWindow();
#endif

	memcpy(adv, adv_data, sizeof(adv_data));

	uint8_t txPower = LL_TX_POWER_0_DBM;
	iSLERInit(txPower);

	idle_ms(200);
	printf(".~ ch32fun BLE beacon ~.\n");

	while(1) {
		// BLE advertisements are sent on channels 37, 38 and 39, over the 1M PHY
		for(int c = 0; c < sizeof(adv_channels); c++) {
			iSLERTX(ACCESS_ADDRESS, adv, sizeof(adv_data), adv_channels[c], PHY_1M);
		}

#if USB_SERIAL_DEBUG
		Delay_Ms(SLEEPTIME_MS);
		poll_input();
#else
		LowPower( MS_TO_RTC(SLEEPTIME_MS), (RB_PWR_RAM2K | RB_PWR_RAMX | RB_PWR_EXTEND) );
		if(usb_plugged) {
			usbBootWindow();
			usb_plugged = 0;
		}
#endif

		iSLERInit(txPower); // RF wakes up in an odd state, we need to reinit after sleep
		DCDCEnable(); // DCDC gets disabled during sleep
		idle_ms(50);
		printf("hello, world!\n");

	}
}
