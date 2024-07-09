/*******************************************************************************
 * Copyright 2019-2024 Microchip Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 *
 */

 /*========================================================================*//**
  @mainpage Configuration for the MiV-IhC driver

    @section intro_sec Introduction
    Used to configure the driver

    @section

*//*==========================================================================*/


#ifndef MIV_IHC_CONFIG_H_
#define MIV_IHC_CONFIG_H_

// #include "miv_ihc_add_mapping.h"
#include "miv_ihc_defines.h"

/*
 * This value not be greater than IP value
 * MAX value in the IP is 4 bytes in currently released IP (Version 6)
 * Note: The function IHC_max_message_size() returns the max value supported by
 * the IP.
 */
#define IHC_MESSAGE_SIZE_IN_BYTES           4U

/*
 * Support pre v6 IHC IP
 * Pre version 6 did not support local interrupts for HSS transmission.
 */
//#define SUPPORT_PRE_LOCAL_HSS_INTS


/*------------------------------------------------------------------------------
 * define the monitor hart (HSS hart) used in our system
 */
#define HSS_HART_MASK               HART0_MASK
#define HSS_HART_ID                 HART0_ID

/*------------------------------------------------------------------------------
 * HSS_REMOTE_HARTS_MASK
 * This is used to define the harts the HSS is communicating with
 */
#define HSS_REMOTE_HARTS_MASK 		(HART1_MASK | HART2_MASK |HART3_MASK | HART4_MASK)

/*------------------------------------------------------------------------------
 * Define which harts are connected via comms channels to a particular hart
 * user defined
 */
#define IHCIA_H0_REMOTE_HARTS	(HSS_REMOTE_HARTS_MASK) /* connected to all harts */
#define IHCIA_H1_REMOTE_HARTS	(HSS_HART_MASK | HART2_MASK | HART3_MASK | HART4_MASK)
#define IHCIA_H2_REMOTE_HARTS	(HSS_HART_MASK | HART1_MASK | HART3_MASK | HART4_MASK)
#define IHCIA_H3_REMOTE_HARTS	(HSS_HART_MASK | HART1_MASK | HART2_MASK | HART4_MASK)
#define IHCIA_H4_REMOTE_HARTS	(HSS_HART_MASK | HART1_MASK | HART2_MASK | HART4_MASK)

/*------------------------------------------------------------------------------
 * interrupts enabled in this system design for a particular hart
 * User defined
 */
#define IHCIA_H0_REMOTE_HARTS_INTS    HSS_HART_DEFAULT_INT_EN  /* connected to all harts */

#define IHCIA_H1_REMOTE_HARTS_INTS    (HSS_HART_MP_INT_EN | HSS_HART_ACK_INT_EN | \
                                      HART2_MP_INT_EN | HART2_ACK_INT_EN | \
                                      HART3_MP_INT_EN | HART3_ACK_INT_EN | \
                                      HART4_MP_INT_EN | HART4_ACK_INT_EN)

#define IHCIA_H2_REMOTE_HARTS_INTS    (HSS_HART_MP_INT_EN | HSS_HART_ACK_INT_EN | \
                                      HART1_MP_INT_EN | HART1_ACK_INT_EN | \
                                      HART3_MP_INT_EN | HART3_ACK_INT_EN | \
                                      HART4_MP_INT_EN | HART4_ACK_INT_EN)

#define IHCIA_H3_REMOTE_HARTS_INTS    (HSS_HART_MP_INT_EN | HSS_HART_ACK_INT_EN | \
                                      HART1_MP_INT_EN | HART1_ACK_INT_EN | \
                                      HART2_MP_INT_EN | HART2_ACK_INT_EN | \
                                      HART4_MP_INT_EN | HART4_ACK_INT_EN)

#define IHCIA_H4_REMOTE_HARTS_INTS    (HSS_HART_MP_INT_EN | HSS_HART_ACK_INT_EN | \
                                      HART1_MP_INT_EN | HART1_ACK_INT_EN | \
                                      HART2_MP_INT_EN | HART2_ACK_INT_EN | \
                                      HART3_MP_INT_EN | HART3_ACK_INT_EN)

#endif /* MIV_IHC_CONFIG_H_ */

