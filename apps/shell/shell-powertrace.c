/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Shell interface to the powertrace app
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "shell.h"
#include "powertrace.h"
#include <stdio.h>
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "contiki-conf.h"

/*---------------------------------------------------------------------------*/
PROCESS(shell_powertrace_process, "powertrace");
SHELL_COMMAND(powertrace_command,
	      "powertrace",
	      "powertrace [interval]: turn powertracing on or off, with reporting interval <interval>",
	      &shell_powertrace_process);
PROCESS(shell_rpl_repair_process, "RPL repair");
SHELL_COMMAND(rpl_repair_command,	      
			"rpl_repair",	      
			"rpl_repair: trigger a global RPL repair",	      
			&shell_rpl_repair_process);
PROCESS(shell_tx_power_setting_process, "Setting new tx power");
SHELL_COMMAND(tx_power_setting_command,	      
			"txpower",	      
			"txpower <new tx power>",	      
			&shell_tx_power_setting_process);


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_powertrace_process, ev, data)
{
  char buf[10];
  int interval;

  PROCESS_BEGIN();

  interval = shell_strtolong(data, NULL);

  if(data == NULL || interval == 0) {
    powertrace_stop();
    powertrace_print("");
  } else {
    powertrace_start(interval * CLOCK_SECOND);
    sprintf(buf, "%d", interval);
    shell_output_str(&powertrace_command, "Starting powertrace with interval ", buf);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_rpl_repair_process, ev, data)
{   	
	PROCESS_BEGIN();    	
	rpl_repair_root(RPL_DEFAULT_INSTANCE);  	
	shell_output(&rpl_repair_command, "Initiaing global repair", 23, "", 0);  	
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_tx_power_setting_process, ev, data)
{   	
	const char *next, *nextptr;
	uint8_t newTxPower;

	PROCESS_BEGIN();
	//next = strchr(data, ' ');
	next = (char *)data;
  	if(next == NULL) {
    	shell_output_str(&tx_power_setting_command,"txpower <new tx power>", "");
    	PROCESS_EXIT();
  	}
	//++next;
	newTxPower = (uint8_t)shell_strtolong(next, &nextptr);
	rf230_set_new_txpower(newTxPower);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

void
shell_powertrace_init(void)
{
  shell_register_command(&powertrace_command);
  shell_register_command(&rpl_repair_command);
  shell_register_command(&tx_power_setting_command);
  powertrace_sniff(POWERTRACE_ON);
}
/*---------------------------------------------------------------------------*/
