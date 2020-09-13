/**
 * 
 * Copyright (c) 2020 Brendan Moran
 * 
 */
#include "qspi_config.h"
#include "fsl_flexspi.h"
#include "fsl_iomuxc.h"
#include "MIMXRT1011.h"


flexspi_device_config_t deviceconfig = {
    .flexspiRootClk       = 133000000,
    .flashSize            = FPGA_MEM_SIZE,
    .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
    .CSInterval           = 2,
    .CSHoldTime           = 3,
    .CSSetupTime          = 3,
    .dataValidTime        = 0,
    .columnspace          = 0,
    .enableWordAddress    = 0,
    .AWRSeqIndex          = FPGA_CMD_LUT_SEQ_IDX_WRITE_FAST_QUAD,
    .AWRSeqNumber         = 1,
    .ARDSeqIndex          = FPGA_CMD_LUT_SEQ_IDX_READ_FAST_QUAD,
    .ARDSeqNumber         = 1,
    .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
    .AHBWriteWaitInterval = 0,
};

const uint32_t customLUT[FPGA_LUT_LENGTH] = {
    /* Read status register */
    [4 * FPGA_CMD_LUT_SEQ_IDX_GET_STATUS] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0x00, kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0x04),

    /* Read ID */
    [4 * FPGA_CMD_LUT_SEQ_IDX_GET_ID] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0x1, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* FIFO Write -SDR */
    [4 * FPGA_CMD_LUT_SEQ_IDX_WRITE_FIFO_FAST_QUAD] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0x10, kFLEXSPI_Command_DATSZ_SDR, kFLEXSPI_4PAD, 0x10),
    [4 * FPGA_CMD_LUT_SEQ_IDX_WRITE_FIFO_FAST_QUAD + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_4PAD, 0),

    /* FIFO Read -SDR */
    [4 * FPGA_CMD_LUT_SEQ_IDX_READ_FIFO_FAST_QUAD] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0x11, kFLEXSPI_Command_DATSZ_SDR, kFLEXSPI_4PAD, 0x10),
    [4 * FPGA_CMD_LUT_SEQ_IDX_READ_FIFO_FAST_QUAD + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0x01, kFLEXSPI_Command_STOP, kFLEXSPI_4PAD, 0),

    /* Fast read quad mode - SDR */
    [4 * FPGA_CMD_LUT_SEQ_IDX_READ_FAST_QUAD] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0x10, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_4PAD, 0x18),
    [4 * FPGA_CMD_LUT_SEQ_IDX_READ_FAST_QUAD + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_4PAD, 0x01, kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0x04),

    /* Fast write quad mode - SDR */
    [4 * FPGA_CMD_LUT_SEQ_IDX_WRITE_FAST_QUAD] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0x11, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_4PAD, 0x18),
    [4 * FPGA_CMD_LUT_SEQ_IDX_WRITE_FAST_QUAD + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_4PAD, 0x01, kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0x04),

};

void flexspi_clock_init(void)
{
#if defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1)
    /* Switch to PLL2 for XIP to avoid hardfault during re-initialize clock. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 24);    /* Set PLL2 PFD2 clock 396MHZ. */
    CLOCK_SetMux(kCLOCK_FlexspiMux, 0x2); /* Choose PLL2 PFD2 clock as flexspi source clock. */
    CLOCK_SetDiv(kCLOCK_FlexspiDiv, 2);   /* flexspi clock 133M. */
#else
    const clock_usb_pll_config_t g_ccmConfigUsbPll = {.loopDivider = 0U};

    CLOCK_InitUsb1Pll(&g_ccmConfigUsbPll);
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 24);   /* Set PLL3 PFD0 clock 360MHZ. */
    CLOCK_SetMux(kCLOCK_FlexspiMux, 0x3); /* Choose PLL3 PFD0 clock as flexspi source clock. */
    CLOCK_SetDiv(kCLOCK_FlexspiDiv, 2);   /* flexspi clock 120M. */
#endif
}

/**
 * Set up the debug SPI configuration.
 */
void debug_spi_init(void)
{

  FLEXSPI_Type *base = FLEXSPI;

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_00_FLEXSPI_B_SS0_B,      /* GPIO_00 is configured as FLEXSPI_B_SS0_B */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_00 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_01_FLEXSPI_B_DATA01,      /* GPIO_SD_01 is configured as FLEXSPI_B_DATA01 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_01 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_02_FLEXSPI_B_DATA02,      /* GPIO_SD_02 is configured as FLEXSPI_B_DATA02 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_02 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_03_FLEXSPI_B_DATA00,     /* GPIO_SD_03 is configured as FLEXSPI_B_DATA0 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_03 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_13_FLEXSPI_B_SCLK,       /* GPIO_SD_13 is configured as FLEXSPI_B_SCLK */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_13 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_04_FLEXSPI_B_DATA03,      /* GPIO_SD_04 is configured as FLEXSPI_A_DATA03 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_04 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_00_FLEXSPI_B_DQS,           /* GPIO_00 is configured as FLEXSPI_B_DQS */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_00 */

  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_01_FLEXSPI_B_DATA01,      /* GPIO_SD_01 PAD functional properties : */
      0x10E1U);                               /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: R0/4
                                                 Speed Field: max(200MHz)
                                                 Open Drain Enable Field: Open Drain Disabled
                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
                                                 Pull / Keep Select Field: Keeper
                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                 Hyst. Enable Field: Hysteresis Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_02_FLEXSPI_B_DATA02,      /* GPIO_SD_02 PAD functional properties : */
      0x10E1U);                               /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: R0/4
                                                 Speed Field: max(200MHz)
                                                 Open Drain Enable Field: Open Drain Disabled
                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
                                                 Pull / Keep Select Field: Keeper
                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                 Hyst. Enable Field: Hysteresis Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_03_FLEXSPI_B_DATA00,     /* GPIO_SD_03 PAD functional properties : */
      0x10E1U);                               /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: R0/4
                                                 Speed Field: max(200MHz)
                                                 Open Drain Enable Field: Open Drain Disabled
                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
                                                 Pull / Keep Select Field: Keeper
                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                 Hyst. Enable Field: Hysteresis Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_13_FLEXSPI_B_SCLK,       /* GPIO_SD_13 PAD functional properties : */
      0x10E1U);                               /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: R0/4
                                                 Speed Field: max(200MHz)
                                                 Open Drain Enable Field: Open Drain Disabled
                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
                                                 Pull / Keep Select Field: Keeper
                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                 Hyst. Enable Field: Hysteresis Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_04_FLEXSPI_B_DATA03,     /* GPIO_SD_04 PAD functional properties : */
      0x10E1U);                               /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: R0/4
                                                 Speed Field: max(200MHz)
                                                 Open Drain Enable Field: Open Drain Disabled
                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
                                                 Pull / Keep Select Field: Keeper
                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                 Hyst. Enable Field: Hysteresis Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_00_FLEXSPI_B_DQS,           /* GPIO_00 PAD functional properties : */
      0x10E1U);                               /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: R0/4
                                                 Speed Field: max(200MHz)
                                                 Open Drain Enable Field: Open Drain Disabled
                                                 Pull / Keep Enable Field: Pull/Keeper Enabled
                                                 Pull / Keep Select Field: Keeper
                                                 Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                 Hyst. Enable Field: Hysteresis Disabled */


	flexspi_clock_init();
	flexspi_config_t config;

	/*Get FLEXSPI default settings and configure the flexspi. */
	FLEXSPI_GetDefaultConfig(&config);

	config.enableSameConfigForAll = false; // Reference Manual 25.6.6

	/*Set AHB buffer size for reading data through AHB bus. */
	config.ahbConfig.enableAHBPrefetch    = false;
	config.ahbConfig.enableAHBBufferable  = true;
	config.ahbConfig.enableReadAddressOpt = true;
	config.ahbConfig.enableAHBCachable    = false;
	config.rxSampleClock                  = kFLEXSPI_ReadSampleClkLoopbackFromDqsPad;
	FLEXSPI_Init(base, &config);

	/* Configure flash settings according to serial flash feature. */
	FLEXSPI_SetFlashConfig(base, &deviceconfig, kFLEXSPI_PortA1);

	/* Update LUT table. */
	FLEXSPI_UpdateLUT(base, 0, customLUT, FPGA_LUT_LENGTH);

	/* Do software reset. */
	FLEXSPI_SoftwareReset(base);

}
