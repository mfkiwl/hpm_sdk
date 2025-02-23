/*
 * Copyright (c) 2022 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "board.h"
#include "pinmux.h"
#include "audio_v1_mic_speaker.h"


int main(void)
{
    board_init();
    board_init_usb_pins();

    board_init_pdm_clock();
    init_pdm_pins();

    board_init_dao_clock();
    init_dao_pins();

    intc_set_irq_priority(IRQn_USB0, 2);
    i2s_enable_dma_irq_with_priority(1);

    printf("cherry usb audio v1 mic speaker sample.\n");

    audio_init();
    mic_init_i2s_pdm();
    speaker_init_i2s_dao();

    while (1) {
        audio_test();
    }
    return 0;
}
