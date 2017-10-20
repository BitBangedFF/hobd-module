/**
 * @file comm.h
 * @brief TODO.
 *
 */

#ifndef COMM_H
#define	COMM_H

#include <stdint.h>

void comm_init(void);

void comm_deinit(void);

void comm_update(void);

void comm_check_timeout_reset(void);

#endif	/* COMM_H */
