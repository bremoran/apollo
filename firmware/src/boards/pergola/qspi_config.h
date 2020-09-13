/**
 * 
 * Copyright (c) 2020 Brendan Moran
 * 
 */

#ifndef QSPI_CONFIG_H
#define QSPI_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    FPGA_CMD_LUT_SEQ_IDX_GET_STATUS = 0,
    FPGA_CMD_LUT_SEQ_IDX_GET_ID,
    FPGA_CMD_LUT_SEQ_IDX_WRITE_FIFO_FAST_QUAD,
    FPGA_CMD_LUT_SEQ_IDX_READ_FIFO_FAST_QUAD,
    FPGA_CMD_LUT_SEQ_IDX_WRITE_FAST_QUAD,
    FPGA_CMD_LUT_SEQ_IDX_READ_FAST_QUAD,
};

#define FPGA_LUT_LENGTH 60
#define FPGA_MEM_SIZE (16*1024)



#ifdef __cplusplus
}
#endif


#endif // QSPI_CONFIG_H
