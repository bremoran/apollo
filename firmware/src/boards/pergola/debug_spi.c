/**
 * Interface code for communicating with the FPGA over the Debug SPI connection.
 *
 * This file is part of LUNA.
 *
 * Copyright (c) 2020 Great Scott Gadgets <info@greatscottgadgets.com>
 * Copyright (c) 2020 Konrad Beckmann
 * Copyright (c) 2020 Brendan Moran
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "qspi_config.h"

#include <tusb.h>
#include <apollo_board.h>
#include <fsl_flexspi.h>
#include <MIMXRT1011.h>


/**
 * Request that sends a block of data over our debug SPI.
 */
bool handle_flash_spi_send(uint8_t rhport, tusb_control_request_t const* request)
{
    // Daisho doesn't support this, due to lack of flash.
    return false;
}

bool handle_flash_spi_send_complete(uint8_t rhport, tusb_control_request_t const* request)
{
    // Daisho doesn't support this, due to lack of flash.
    return false;
}

extern uint8_t spi_in_buffer[];
extern uint8_t spi_out_buffer[];

bool handle_debug_spi_send_complete(uint8_t rhport, tusb_control_request_t const* request)
{
    flexspi_transfer_t xfer = {
        .deviceAddress = 0,
        .port          = kFLEXSPI_PortB1,
        .cmdType       = kFLEXSPI_Write,
        .SeqNumber     = 1,
        .seqIndex      = FPGA_CMD_LUT_SEQ_IDX_WRITE_FIFO_FAST_QUAD,
        .data          = &spi_out_buffer,
        .dataSize      = request->wLength,

    };
    status_t rc = FLEXSPI_TransferBlocking(
        FLEXSPI,
        &xfer
    );
    if (rc == kStatus_Success) {
        // TODO: Read status register first
        xfer.data = &spi_in_buffer;
        xfer.cmdType = kFLEXSPI_Read;
        xfer.seqIndex = FPGA_CMD_LUT_SEQ_IDX_READ_FIFO_FAST_QUAD,
        rc = FLEXSPI_TransferBlocking(
            FLEXSPI,
            &xfer
        );
    }


    return true;
}
