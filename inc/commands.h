/*
 * comamnds.h
 *
 *  Created on: 11 gru 2018
 *      Author: sobriodev
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "chip.h"

#define COMMANDS_COUNT	1
#define COMMMAND_LEN 	2

/*!
 * @brief Command structure
 */
typedef struct {
	/*! Command name base address */
	const char *name;
	/*! Command callback base address */
	void(*callback)(void);
} command;

/*!
 * @brief Invoke input command
 * @param[in] cmd : Input command
 * @return True if command was found and invoked, false otherwise
 */
bool invoke_cmd(const char *cmd);

#endif /* COMMANDS_H_ */
