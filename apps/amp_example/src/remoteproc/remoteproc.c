/*******************************************************************************
 * Copyright 2019-2022 Microchip Embedded Systems Solutions..
 *
 * SPDX-License-Identifier: MIT
 *
 *
 * This file provides helper functions to enable the ihc communication channel.
 * This allows to receive control messages (start, stop, etc) from Linux without
 * the need of having rpmsg enabled.
 */

#include <stdio.h>
#include <string.h>
#include "mss_extra_sw_config.h"
#include "miv_ihc.h"
#include "remoteproc.h"
#include "rpmsg_platform.h"

#ifdef USING_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#elif defined(__ZEPHYR__)
#include <zephyr/kernel.h>
#else
#include "utils.h"
#endif

typedef void (*func_t)(void);
extern const uint64_t __SCRATCHPAD_start;

enum miv_rp_mbox_messages {
    MIV_RP_MBOX_READY = 0xFFFFFF00,
    MIV_RP_MBOX_PENDING_MSG = 0xFFFFFF01,
    MIV_RP_MBOX_STOP = 0xFFFFFF02,
    MIV_RP_MBOX_END_MSG = 0xFFFFFF03,
};

/* all needed registration are done in rpmsg lite*/
void rproc_setup(uint32_t link_id){
    return;
}

void rproc_stop(uint32_t channel)
{
    switch (channel)
    {
        case RL_PLATFORM_MIV_IHC_CH0_ID:
        case RL_PLATFORM_MIV_IHC_CH1_ID:
        case RL_PLATFORM_MIV_IHC_CH2_ID:
        case RL_PLATFORM_MIV_IHC_CH3_ID:
        case RL_PLATFORM_MIV_IHC_CH4_ID:
            irq_disable(DT_IRQN_BY_IDX(DT_NODELABEL(ihc), 0));
            break;
        case RL_PLATFORM_MIV_IHC_CH5_ID:
        case RL_PLATFORM_MIV_IHC_CH6_ID:
        case RL_PLATFORM_MIV_IHC_CH7_ID:
        case RL_PLATFORM_MIV_IHC_CH8_ID:
        case RL_PLATFORM_MIV_IHC_CH9_ID:
            irq_disable(DT_IRQN_BY_IDX(DT_NODELABEL(ihc), 1));
            break;
        case RL_PLATFORM_MIV_IHC_CH10_ID:
        case RL_PLATFORM_MIV_IHC_CH11_ID:
        case RL_PLATFORM_MIV_IHC_CH12_ID:
        case RL_PLATFORM_MIV_IHC_CH13_ID:
        case RL_PLATFORM_MIV_IHC_CH14_ID:
            irq_disable(DT_IRQN_BY_IDX(DT_NODELABEL(ihc), 2));
            break;
        case RL_PLATFORM_MIV_IHC_CH15_ID:
        case RL_PLATFORM_MIV_IHC_CH16_ID:
        case RL_PLATFORM_MIV_IHC_CH17_ID:
        case RL_PLATFORM_MIV_IHC_CH18_ID:
        case RL_PLATFORM_MIV_IHC_CH19_ID:
            irq_disable(DT_IRQN_BY_IDX(DT_NODELABEL(ihc), 3));
            break;
        case RL_PLATFORM_MIV_IHC_CH20_ID:
        case RL_PLATFORM_MIV_IHC_CH21_ID:
        case RL_PLATFORM_MIV_IHC_CH22_ID:
        case RL_PLATFORM_MIV_IHC_CH23_ID:
        case RL_PLATFORM_MIV_IHC_CH24_ID:
            irq_disable(DT_IRQN_BY_IDX(DT_NODELABEL(ihc), 4));
            break;
        default:
            /* All the cases have been listed above, the default clause should not be reached. */
            break;
    }

    // use that trick to call the function in the scratchpad with -mo-pie activated
    __asm__ volatile ("jalr ra, 0(%0)\n\t"
    :: "r" (__SCRATCHPAD_start):"ra");
}
