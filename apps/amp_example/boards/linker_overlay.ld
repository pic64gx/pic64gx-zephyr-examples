/* linker.ld - Linker command/script file
 *
 * Copyright (c) 2023 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

.resource_table : ALIGN(0x10)
{
	KEEP (*(.resource_table*))
} > RAM
