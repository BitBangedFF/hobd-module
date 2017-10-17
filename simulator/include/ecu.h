/**
 * @file ecu.h
 * @brief TODO.
 *
 */

#ifndef ECU_H
#define	ECU_H

#include <stdint.h>

void ecu_init(void);

void ecu_deinit(void);

void ecu_update(void);

void ecu_check_timeout_reset(void);

#endif	/* ECU_H */
