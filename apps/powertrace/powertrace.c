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
 *         Powertrace: periodically print out power consumption
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "sys/compower.h"
#include "powertrace.h"
#include "net/rime/rime.h"
#include "net/netstack.h"

#define DEBUG   DEBUG_NONE
#define PRINTF_SP_YIBO 0

#include "net/ip/uip-debug.h"

#include <stdio.h>
#include <string.h>

struct powertrace_sniff_stats {
  struct powertrace_sniff_stats *next;
  uint32_t num_input, num_output;
  uint32_t input_txtime, input_rxtime;
  uint32_t output_txtime, output_rxtime;
#if NETSTACK_CONF_WITH_IPV6
  uint16_t proto; /* includes proto + possibly flags */
#endif
  uint16_t channel;
  uint32_t last_input_txtime, last_input_rxtime;
  uint32_t last_output_txtime, last_output_rxtime;
};

#define INPUT  1
#define OUTPUT 0

#define MAX_NUM_STATS  16

//YIBO: calculation of parameters like those in collect-view java program
#define TICKS_PER_SECOND 7812L
#define VOLTAGE 3300UL 			//YIBO: mV
#define POWER_CPU 8250UL       /* mW 2.5 * VOLTAGE*/
#define POWER_LPM 264UL      /* mW 0.08 * VOLTAGE*/
#define POWER_TRANSMIT  47850UL  /* mW 14.5 * VOLTAGE*/
#define POWER_LISTEN 41250UL     /* mW 12.5 * VOLTAGE*/


MEMB(stats_memb, struct powertrace_sniff_stats, MAX_NUM_STATS);
LIST(stats_list);

PROCESS(powertrace_process, "Periodic power output");
/*---------------------------------------------------------------------------*/
uint32_t remainder(uint32_t dividend, uint32_t divisor)
{
	uint32_t quotient, remainder;
	quotient = dividend/divisor;
	remainder = dividend%divisor;
	if (quotient>=1)
		if (remainder >= divisor/2)
			return 1;
		else
			return 0;
	else if (quotient == 0)
		if (quotient >= 1/2)
			return 1;
		else
			return 0;
	else
		return 0;
}
/*---------------------------------------------------------------------------*/
void
powertrace_print(char *str)
{
  //YIBO: Be careful the static variables, it will apply their memory before the program starts running.
  //YIBO: Like the static power snapshot !!
  
  int powermode = appMessage.data.scm_sensors.powermode_yibo;
  static uint32_t powerchange;
  static uint32_t last_cpu, last_lpm, last_transmit, last_listen;
  static uint32_t last_idle_transmit, last_idle_listen;

  uint32_t cpu, lpm, transmit, listen;
  uint32_t all_cpu, all_lpm, all_transmit, all_listen;
  uint32_t idle_transmit, idle_listen;
  uint32_t all_idle_transmit, all_idle_listen;

  static uint32_t seqno;

  uint32_t time, all_time, radio, all_radio;
  
  struct powertrace_sniff_stats *s;

  energest_flush();

  all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  all_idle_transmit = compower_idle_activity.transmit;
  all_idle_listen = compower_idle_activity.listen;

  cpu = all_cpu - last_cpu;
  lpm = all_lpm - last_lpm;
  transmit = all_transmit - last_transmit;
  listen = all_listen - last_listen;
  idle_transmit = compower_idle_activity.transmit - last_idle_transmit;
  idle_listen = compower_idle_activity.listen - last_idle_listen;

  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  last_idle_listen = compower_idle_activity.listen;
  last_idle_transmit = compower_idle_activity.transmit;

  radio = transmit + listen;
  time = cpu + lpm;
  all_time = all_cpu + all_lpm;
  all_radio = energest_type_time(ENERGEST_TYPE_LISTEN) +
    energest_type_time(ENERGEST_TYPE_TRANSMIT);

  	/*YIBO: First line info about the time of energy consumption*/
  	/* In the example of udp, every three minutes, each node will print one time of PowerTrace information.
  	*	1.	str = "#P" from udp-client.c
  	*	2.	clock_time(): the timer depends on different hardware platform
  	*	"P": content after "P"
  	*	3+4:	rimeaddr_node_addr.u8[0].rimeaddr_node_addr.u8[1]: first and second bytes of the Rime address. <Not work in Wismote>
  	*	5.	seqno: after each time's printing, seqno++
  	*	6.	all_cpu: energest_type_time(ENERGEST_TYPE_CPU)
  	*	7.	all_lpm: energest_type_time(ENERGEST_TYPE_LPM)
  	*	8.	all_transmit: energest_type_time(ENERGEST_TYPE_TRANSMIT)
  	*	9.	all_listen: energest_type_time(ENERGEST_TYPE_LISTEN)
  	*	10.	all_idle_transmit: compower_idle_activity.transmit
  	*	11.	all_idle_listen: compower_idle_activity.listen
  	*	12.	cpu: all_cpu - last_cpu
  	*	13.	lpm: all_lpm - last_lpm
  	*	14.	transmit: all_transmit - last_transmit
  	*	15.	listen = all_listen - last_listen
  	*	16.	idle_transmit = compower_idle_activity.transmit - last_idle_transmit
  	*	17.	idle_listen = compower_idle_activity.listen - last_idle_listen
  	*	in the (radio  /  tx  /  listen  / )
  	*	18.	
    	*/
  	/*
	01:01.192	ID:3	#P 7687 P [0.18] 0		<1--5> 
	(67685 1898660 22671 20235 0 13074)		<6--11>
	(67685 1898660 22671 20235 0 13074)		<12--17>
	(radio 2.18% / 2.18% tx 1.15% / 1.15% listen 1.02% / 1.02%)
	01:01.206	ID:3	#P 7691 SP 0.18 0 58 39680 1 0 19 0 19 0 0 0 0 0 (proto 58(39680) radio 0.04% / 0.04%)
	01:01.224	ID:3	#P 7692 SP 0.18 0 0 0 0 0 0 0 0 11 22671 4210 22671 4210 (proto 0(0) radio 62.6265% / 62.6265%)
	01:01.239	ID:3	#P 7695 SP 0.18 0 58 39681 12 0 2932 0 2932 0 0 0 0 0 (proto 58(39681) radio 6.683% / 6.683%)

	*/
	/*YIBO: test power left of the battery*/
  	printf("Test PL PI PC [%lu, %lu, %lu], 4P (%lu, %lu, %lu, %lu)\n",
			(uint32_t)(powerleft),
         	(uint32_t)(powerInitial),
         	(uint32_t)(((cpu / TICKS_PER_SECOND) * (POWER_CPU / 1000)) + 
         				//YIBO: cpu tick number maybe very large (cpu * POWER_CPU)/(1000L * TICKS_PER_SECOND) + 
         				 (lpm * POWER_LPM)/(1000L * TICKS_PER_SECOND) + 
    					 //((listen / TICKS_PER_SECOND) * (POWER_LISTEN / 1000)) + 
    					 (listen * POWER_LISTEN)/(1000L * TICKS_PER_SECOND) +
    					 (transmit * POWER_TRANSMIT)/(1000L * TICKS_PER_SECOND)),
			(uint32_t)((cpu * POWER_CPU)/(1000L * TICKS_PER_SECOND)),
			(uint32_t) ((lpm * POWER_LPM)/(1000L * TICKS_PER_SECOND)),
    		//(uint32_t) ((listen / TICKS_PER_SECOND) * (POWER_LISTEN / 1000))
    		(uint32_t) ((listen * POWER_LISTEN)/(1000L * TICKS_PER_SECOND)),
    		(uint32_t) ((transmit * POWER_TRANSMIT)/(1000L * TICKS_PER_SECOND))
    	   );
	
	if (!powermode){ //YIBO: dealling with power mode == 0
		powerchange += (uint32_t)(((cpu / TICKS_PER_SECOND) * (POWER_CPU / 1000) + 
         					(lpm * POWER_LPM)/(1000L * TICKS_PER_SECOND) + 
    						//((listen / TICKS_PER_SECOND) * (POWER_LISTEN / 1000)) + 
    						(listen * POWER_LISTEN)/(1000L * TICKS_PER_SECOND) +
    						(transmit * POWER_TRANSMIT)/(1000L * TICKS_PER_SECOND)));
		powerchange += (remainder(cpu, TICKS_PER_SECOND) + 
				   	remainder(lpm * POWER_LPM, 1000L * TICKS_PER_SECOND) +
				   	remainder(listen * POWER_LISTEN, 1000L * TICKS_PER_SECOND) +
				   	remainder(transmit * POWER_TRANSMIT, 1000L * TICKS_PER_SECOND));
	}
	else if (powermode == 1)
		powerchange += 0;
	else if (powermode == 2)
		powerchange -= (uint32_t)(powerInitial/100);
	else 
		printf("Unknown Power Mode.\n");
	
	if (powerleft > 0 && powerleft <= powerInitial){
		if (powermodif == 0){
			powerleft = powerInitial - powerchange;
			if (powerleft <= 0 || powerleft > powerInitial){
				powerleft = 0;
				powertrace_stop();
			}
		}
		else if (powermodif > 0){ //YIBO: the power left is increased by shell command
			powerchange += (uint32_t)powermodif;
			powerleft = powerInitial - powerchange;
			powermodif = 0;
		}
		else if (powermodif < 0){ //YIBO: the power left is reduced by shell command
			powerchange -= (uint32_t)(powermodif*(-1));
			powerleft = powerInitial - powerchange;
			powermodif = 0;
		}						
	}else{
		powerleft = 0;
		printf("I am Dead finally. \n");
	}

	if(powerleft > 0){
		ANNOTATE("#A PowLeft=%d.%02d%%\n", 
					(int) ((100L * (powerleft/10)) / (powerInitial/10)), 
         			(int)((10000L * (powerleft/1000)/ (powerInitial/1000)) - ((100L * (powerleft/1000)/ (powerInitial/1000))) * 100)
         		); //because powerleft/ powerInitial < 1, we multiple 100L here
  	}
  	else{
		ANNOTATE("#A PowLeft=ZERO\n");
		//powerleft = 0;  //YIBO: Have to be reassigned again, otherwise, battery indicator cannot be presented correctly...
		//powertrace_stop();  //YIBO: very curious whether this process can be exited...shell process cannot...
	}

#if PRINTF_SP_YIBO == 1
  printf("%s %lu P PL=%lu [%d.%d] %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu (radio %d.%02d%% / %d.%02d%% tx %d.%02d%% / %d.%02d%% listen %d.%02d%% / %d.%02d%%)\n",
         str,
         clock_time(), 
         powerleft,
         linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7], seqno,
         all_cpu, all_lpm, all_transmit, all_listen, all_idle_transmit, all_idle_listen,
         cpu, lpm, transmit, listen, idle_transmit, idle_listen,
         (int)((100L * (all_transmit + all_listen)) / all_time),
         (int)((10000L * (all_transmit + all_listen) / all_time) - (100L * (all_transmit + all_listen) / all_time) * 100),
         (int)((100L * (transmit + listen)) / time),
         (int)((10000L * (transmit + listen) / time) - (100L * (transmit + listen) / time) * 100),
         (int)((100L * all_transmit) / all_time),
         (int)((10000L * all_transmit) / all_time - (100L * all_transmit / all_time) * 100),
         (int)((100L * transmit) / time),
         (int)((10000L * transmit) / time - (100L * transmit / time) * 100),
         (int)((100L * all_listen) / all_time),
         (int)((10000L * all_listen) / all_time - (100L * all_listen / all_time) * 100),
         (int)((100L * listen) / time),
         (int)((10000L * listen) / time - (100L * listen / time) * 100));
#endif //PRINTF_SP_YIBO == 1

  for(s = list_head(stats_list); s != NULL; s = list_item_next(s)) {

#if ! NETSTACK_CONF_WITH_IPV6
    printf("%s %lu SP %d.%d %lu %u %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu (channel %d radio %d.%02d%% / %d.%02d%%)\n",
           str, clock_time(), linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], seqno,
           s->channel,
           s->num_input, s->input_txtime, s->input_rxtime,
           s->input_txtime - s->last_input_txtime,
           s->input_rxtime - s->last_input_rxtime,
           s->num_output, s->output_txtime, s->output_rxtime,
           s->output_txtime - s->last_output_txtime,
           s->output_rxtime - s->last_output_rxtime,
           s->channel,
           (int)((100L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((100L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio));
#else
#if PRINTF_SP_YIBO == 1
    printf("%s %lu SP %d.%d %lu %u %u %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu (proto %u(%u) radio %d.%02d%% / %d.%02d%%)\n",
           str, clock_time(), linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], seqno,
           s->proto, s->channel,
           s->num_input, s->input_txtime, s->input_rxtime,
           s->input_txtime - s->last_input_txtime,
           s->input_rxtime - s->last_input_rxtime,
           s->num_output, s->output_txtime, s->output_rxtime,
           s->output_txtime - s->last_output_txtime,
           s->output_rxtime - s->last_output_rxtime,
           s->proto, s->channel,
           (int)((100L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((100L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio));
#endif //PRINTF_SP_YIBO == 1

#endif
    s->last_input_txtime = s->input_txtime;
    s->last_input_rxtime = s->input_rxtime;
    s->last_output_txtime = s->output_txtime;
    s->last_output_rxtime = s->output_rxtime;
    
  }
  seqno++;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(powertrace_process, ev, data)
{
  static struct etimer periodic;
  clock_time_t *period;
  PROCESS_BEGIN();

  period = data;

  if(period == NULL) {
    PROCESS_EXIT();
  }
  etimer_set(&periodic, *period);

  while(1) {
    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
    etimer_reset(&periodic);
    powertrace_print("");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
powertrace_start(clock_time_t period)
{
  process_start(&powertrace_process, (void *)&period);
}
/*---------------------------------------------------------------------------*/
void
powertrace_stop(void)
{
  process_exit(&powertrace_process);
}
/*---------------------------------------------------------------------------*/
static void
add_stats(struct powertrace_sniff_stats *s, int input_or_output)
{
  if(input_or_output == INPUT) {
    s->num_input++;
    s->input_txtime += packetbuf_attr(PACKETBUF_ATTR_TRANSMIT_TIME);
    s->input_rxtime += packetbuf_attr(PACKETBUF_ATTR_LISTEN_TIME);
  } else if(input_or_output == OUTPUT) {
    s->num_output++;
    s->output_txtime += packetbuf_attr(PACKETBUF_ATTR_TRANSMIT_TIME);
    s->output_rxtime += packetbuf_attr(PACKETBUF_ATTR_LISTEN_TIME);
  }
}
/*---------------------------------------------------------------------------*/
static void
add_packet_stats(int input_or_output)
{
  struct powertrace_sniff_stats *s;

  /* Go through the list of stats to find one that matches the channel
     of the packet. If we don't find one, we allocate a new one and
     put it on the list. */
  for(s = list_head(stats_list); s != NULL; s = list_item_next(s)) {
    if(s->channel == packetbuf_attr(PACKETBUF_ATTR_CHANNEL)
#if NETSTACK_CONF_WITH_IPV6
       && s->proto == packetbuf_attr(PACKETBUF_ATTR_NETWORK_ID)
#endif
       ) {
      add_stats(s, input_or_output);
      break;
    }
  }
  if(s == NULL) {
    s = memb_alloc(&stats_memb);
    if(s != NULL) {
      memset(s, 0, sizeof(struct powertrace_sniff_stats));
      s->channel = packetbuf_attr(PACKETBUF_ATTR_CHANNEL);
#if NETSTACK_CONF_WITH_IPV6
      s->proto = packetbuf_attr(PACKETBUF_ATTR_NETWORK_ID);
#endif
      list_add(stats_list, s);
      add_stats(s, input_or_output);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
input_sniffer(void)
{
  add_packet_stats(INPUT);
}
/*---------------------------------------------------------------------------*/
static void
output_sniffer(int mac_status)
{
  add_packet_stats(OUTPUT);
}
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_RIME
static void
sniffprint(char *prefix, int seqno)
{
  const linkaddr_t *sender, *receiver, *esender, *ereceiver;

  sender = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  receiver = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  esender = packetbuf_addr(PACKETBUF_ADDR_ESENDER);
  ereceiver = packetbuf_addr(PACKETBUF_ADDR_ERECEIVER);


  printf("%lu %s %d %u %d %d %d.%d %u %u\n",
         clock_time(),
         prefix,
         linkaddr_node_addr.u8[0], seqno,
         packetbuf_attr(PACKETBUF_ATTR_CHANNEL),
         packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE),
         esender->u8[0], esender->u8[1],
         packetbuf_attr(PACKETBUF_ATTR_TRANSMIT_TIME),
         packetbuf_attr(PACKETBUF_ATTR_LISTEN_TIME));
}
/*---------------------------------------------------------------------------*/
static void
input_printsniffer(void)
{
  static int seqno = 0; 
  sniffprint("I", seqno++);

  if(packetbuf_attr(PACKETBUF_ATTR_CHANNEL) == 0) {
    int i;
    uint8_t *dataptr;

    printf("x %d ", packetbuf_totlen());
    dataptr = packetbuf_hdrptr();
    printf("%02x ", dataptr[0]);
    for(i = 1; i < packetbuf_totlen(); ++i) {
      printf("%02x ", dataptr[i]);
    }
    printf("\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
output_printsniffer(int mac_status)
{
  static int seqno = 0;
  sniffprint("O", seqno++);
}
/*---------------------------------------------------------------------------*/
RIME_SNIFFER(printsniff, input_printsniffer, output_printsniffer);
/*---------------------------------------------------------------------------*/
void
powertrace_printsniff(powertrace_onoff_t onoff)
{
  switch(onoff) {
  case POWERTRACE_ON:
    rime_sniffer_add(&printsniff);
    break;
  case POWERTRACE_OFF:
    rime_sniffer_remove(&printsniff);
    break;
  }
}
#endif /* NETSTACK_CONF_WITH_RIME */
/*---------------------------------------------------------------------------*/
RIME_SNIFFER(powersniff, input_sniffer, output_sniffer);
/*---------------------------------------------------------------------------*/
void
powertrace_sniff(powertrace_onoff_t onoff)
{
  switch(onoff) {
  case POWERTRACE_ON:
    rime_sniffer_add(&powersniff);
    break;
  case POWERTRACE_OFF:
    rime_sniffer_remove(&powersniff);
    break;
  }
}

