/*
 * telescope_commands.h
 *
 *  Created on: 26 mars 2019
 *      Author: JLH
 */

#ifndef TELESCOPE_COMMANDS_H_
#define TELESCOPE_COMMANDS_H_

#include "includes.h"

void Process_Receive_Cmd();
void Process_Exec_Cmd(char * cmd);
void Process_Exec_CmdV2(char * cmd);

#endif /* TELESCOPE_COMMANDS_H_ */
