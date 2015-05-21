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
 */

/**
 * \file
 *         Example of how the collect primitive works.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"
#include "serial-shell.h"
#include "collect-view.h"


#include "net/rime/rime.h"
#include "net/rime/timesynch.h"

#include "lib/random.h"
#include "net/netstack.h"
#include "dev/serial-line.h"
#include "dev/rs232.h"

#include "dev/leds.h"
#include "collect-common.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static unsigned long time_offset;
static int send_active = 1;


unsigned short PERIOD_ASSIGN = 60;
unsigned short PERIOD_YIBO = 0;

extern uint8_t rf230_last_rssi;

/*---------------------------------------------------------------------------*/
PROCESS(collect_common_process, "collect common process");
PROCESS(yibo_shell_process, "Contiki yibo shell");
//AUTOSTART_PROCESSES(&collect_common_process);
AUTOSTART_PROCESSES(&collect_common_process, &yibo_shell_process);
/*---------------------------------------------------------------------------*/
static unsigned long
get_time(void)
{
  return clock_seconds() + time_offset;
}
/*---------------------------------------------------------------------------*/
static unsigned long
strtolong(const char *data) {
  unsigned long value = 0;
  int i;
  for(i = 0; i < 10 && isdigit(data[i]); i++) {
    value = value * 10 + data[i] - '0';
  }
  return value;
}
/*---------------------------------------------------------------------------*/
void
collect_common_set_send_active(int active)
{
  send_active = active;
}
/*---------------------------------------------------------------------------*/
void
collect_common_recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops,
                    uint8_t *payload, uint16_t payload_len)
{
  unsigned long time;
  uint16_t data;
  int i;

  uint32_t tempPowerLeft;

  printf("%u", 8 + payload_len / 2);
  /* Timestamp. Ignore time synch for now. */
  time = get_time();
  printf(" %lu %lu 0", ((time >> 16) & 0xffff), time & 0xffff);
  /* Ignore latency for now */
  printf(" %u %u %u %u",
         originator->u8[0] + (originator->u8[1] << 8), seqno, hops, 0);

  if (payload_len >= 18)
  	for(i = 0; i < payload_len / 2; i++) {
    	memcpy(&data, payload, sizeof(data));
    	payload += sizeof(data);
    	printf(" %u", data);
  	}
  else if (payload_len == 4){
  	memcpy(&tempPowerLeft, payload, sizeof(tempPowerLeft));
	printf(" Power left: %lu", tempPowerLeft);
  }	
  else
  	printf(" Command ACK: %s ", payload);
  printf("\n");

  
  //30 0 287 0 3 18 1 0 
  //len=22, clock=34952, timesynch_time=0,  cpu=0, lpm=0, transmit=25 , listen=24496, parent=1, parent_etx=128,
  //current_rtmetric=478, num_neighbors=4, beacon_interval=262, sensor[10]=0 0 0 0 0 0 0 0 0 0
  //leds_blink();
}
/*---------------------------------------------------------------------------*/
void 
change_collecting_frequency(uint8_t freq)
{
	PERIOD_YIBO = freq;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(collect_common_process, ev, data)
{
  static struct etimer period_timer, wait_timer;
  PROCESS_BEGIN();

  collect_common_net_init();

  /* Send a packet every 60-62 seconds. */
  etimer_set(&period_timer, CLOCK_SECOND * PERIOD_ASSIGN);
  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_TIMER) {
      if(data == &period_timer) { 
		
		if(PERIOD_YIBO == 0){
			etimer_reset(&period_timer);
        	etimer_set(&wait_timer, random_rand() % (CLOCK_SECOND * PERIOD_ASSIGN));
		}
		else{
			etimer_set(&period_timer, CLOCK_SECOND * PERIOD_YIBO);
			etimer_set(&wait_timer, random_rand() % (CLOCK_SECOND * PERIOD_YIBO));
			PERIOD_ASSIGN = PERIOD_YIBO;
			PERIOD_YIBO = 0;
		}
			
      } else if(data == &wait_timer) {
        if(send_active) {
          /* Time to send the data */
          collect_common_send();
        }
		else
		  printf("*Too bad, one miss*");
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(yibo_shell_process, ev, data)
{
  PROCESS_BEGIN();

  serial_shell_init();

  shell_powertrace_init();
  shell_text_init();
  shell_time_init();
  shell_udpsend_init();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
