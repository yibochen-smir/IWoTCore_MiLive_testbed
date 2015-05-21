/*
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
 * udpsend fe80::ff:fe00:2 8775 5688
 *
 */

#include "contiki.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "net/rpl/rpl.h"
#include "net/netstack.h"
#include "dev/serial-line.h"
/*
#if CONTIKI_TARGET_Z1
#include "dev/uart0.h"
#else
#include "dev/uart1.h"
#endif
*/
#if iwotcore_CYB
#include "iwotcore_gpio.h"
#endif

#include "dev/rs232.h"
#include "dev/leds.h"


#include "collect-common.h"
#include "collect-view.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define UDP_CLIENT_PORT 8775
#define UDP_SERVER_PORT 5688

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

//YIBO: for checking the power consumption, we bring powertrace.h here
#include "powertrace.h"
uint32_t powerInitial = 594000; //59400000; //mJ
uint32_t powerleft = 594000; //59400000; //mJ, 59400mJ is only enough for 2 hours-work
int powermodif = 0;

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

/*---------------------------------------------------------------------------*/
#if iwotcore_CYB_nano
//YIBO: For controlling Nano reading...
unsigned short NANOPERIOD = 60;

PROCESS(sensors_process_nano, "Sensor Process Nano");
#endif
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
#if iwotcore_CYB_nano
AUTOSTART_PROCESSES(&udp_client_process, &collect_common_process, &sensors_process_nano, &yibo_shell_process);
#else
AUTOSTART_PROCESSES(&udp_client_process, &collect_common_process, &yibo_shell_process);
#endif
/*---------------------------------------------------------------------------*/
void
collect_common_set_sink(void)
{
  /* A udp client can never become sink */
}
/*---------------------------------------------------------------------------*/

void
collect_common_net_print(void)
{
  rpl_dag_t *dag;
  uip_ds6_route_t *r;

  /* Let's suppose we have only one instance */
  dag = rpl_get_any_dag();
  if(dag->preferred_parent != NULL) {
    PRINTF("Preferred parent: ");
    PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));
    PRINTF("\n");
  }
  for(r = uip_ds6_route_head();
      r != NULL;
      r = uip_ds6_route_next(r)) {
    PRINT6ADDR(&r->ipaddr);
  }
  PRINTF("---\n");
}
/*---------------------------------------------------------------------------*/
#include <ctype.h>
unsigned long
strtolong_yibo(const char *data) {
  unsigned long value = 0;
  int i;
  for(i = 0; i < 3 && isdigit(data[i])&&data[i]!=0; i++) {
    value = value * 10 + data[i] - '0';
  }
  return value;
}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
	char *appdata;
	
	char ledCommand[10] = "blink led";
	ledCommand[9] = 0;
	char offLedCommand[10] = "close led";
	ledCommand[9] = 0;

	//YIBO: Command - Power left reply
	char powerLeftCommand[10] = "powerleft";
	powerLeftCommand[9] = 0;
	struct {
    	uint8_t trash1;
    	uint8_t trash2;
    	uint32_t powerleft_t;
  	} msg;
	
	//YIBO: Command - Power left modification -10P
	char powerLeftModifCommandMinus10P[10] = "pwminus10";
	powerLeftModifCommandMinus10P[9] = 0;
	
	//YIBO: Command - Power left modification +5P
	char powerLeftModifCommandPlus5P[8] = "pwplus5";
	powerLeftModifCommandPlus5P[7] = 0;
	
	//YIBO: Command - immediate collect data
	char immediateCollectCommand[10] = "imcollect";
	immediateCollectCommand[9] = 0;
	
	//YIBO: Command - stop reading nano -> system reboot and nano counter increase... 
	char stopReadNanoCommand[13] = "stopreadnano";
	stopReadNanoCommand[12] = 0;
	
	//YIBO: Command - recovery nano reading -> nano counter increase...stop, but message may not be received.
	char recoReadNanoCommand[13] = "recoreadnano";
	recoReadNanoCommand[12] = 0;

	//YIBO: Command - power mode modification ->  
	//YIBO: 0->battery powered, 1->solar panel powered, 2->batter charging and solar panel powered
	char batteryPowerCommand[11] = "powermode0";
	batteryPowerCommand[10] = 0;
	char solarPanelPowerCommand[11] = "powermode1";
	solarPanelPowerCommand[10] = 0;
	char batteryChargeCommand[11] = "powermode2";
	batteryChargeCommand[10] = 0;
	int16_t powermode_t = 0;

	//YIBO: Command - Collecting frequency modification
	char fm10Command[5] = "fm10";
	fm10Command[4] = 0;
	char fm15Command[5] = "fm15";
	fm15Command[4] = 0;
	char fm30Command[5] = "fm30";
	fm30Command[4] = 0;
	char fm60Command[5] = "fm60";
	fm60Command[4] = 0;
	char fm120Command[7] = "fm2min";
	fm120Command[6] = 0;

	//YIBO: Command - tx power modification
	char txpowerCommand[8] = "txpower";
	txpowerCommand[7] = 0;
	char txpowerCommand_temp[8];
	uint8_t txpowermodif;
	const char *next_tx;
	
	if(uip_newdata()) {
	    appdata = (char *)uip_appdata;
    	appdata[uip_datalen()] = 0;
    	PRINTF("DATA recv '%s' from ", appdata);
    	PRINTF("%d",
           	UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
    	PRINTF("\n");

		//YIBO: Command Check - Leds on
		if(strcmp(appdata, ledCommand)==0){
			//PRINTF("Receive Led Command.\n");
			#if iwotcore_CYB
				IWOTCORE_SENSOR_On();
			#else
				leds_on(LEDS_RED);
			#endif
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  Reply ACK", sizeof("  Reply ACK"));
		}
		
		//YIBO: Command Check - Leds off
		if(strcmp(appdata, offLedCommand)==0){
			//PRINTF("Receive Led Command.\n");
			#if iwotcore_CYB
				IWOTCORE_SENSOR_Off();
			#else
				leds_off(LEDS_RED);
			#endif
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  Reply ACK", sizeof("  Reply ACK"));
		}

		//YIBO: Command Check - Power left reply
		if(strcmp(appdata, powerLeftCommand)==0){
			PRINTF("Receive [Power left reply] Command.\n");
			memset(&msg, 0, sizeof(msg));
			powertrace_print("#YB"); //YIBO: For getting latest power left value

			msg.powerleft_t = powerleft;
			uip_udp_packet_sendto(client_conn, &msg, sizeof(msg),
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
		}
		//YIBO: Command Check - Power left modification -10 Percent
		//udpsend fe80::ff:fe00:2 8775 5688
		//pwminus10
		if(strcmp(appdata, powerLeftModifCommandMinus10P)==0){
			PRINTF("Receive [Power left modification -10] Command.\n");
			powermodif = (int)(powerInitial/10);
			powertrace_print("#YB");

			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK -10", sizeof("  ACK -10"));
		}

		if(strcmp(appdata, powerLeftModifCommandPlus5P)==0){
			PRINTF("Receive [Power left modification +5] Command.\n");
			powermodif = ((int)(powerInitial/20))*(-1);
			powertrace_print("#YB");

			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK +5", sizeof("  ACK +5"));
		}

		if(strcmp(appdata, immediateCollectCommand)==0){
			PRINTF("Receive [immediate collect data] Command.\n");
			collect_common_send();
		}

		//YIBO: We have two commands to control the NANO module
		if(strcmp(appdata, stopReadNanoCommand)==0){
			PRINTF("Receive [stop read nano] Command.\n");
			NANOPERIOD = 180;

			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK Stop Nano", sizeof("  ACK Stop Nano"));
		}

		if(strcmp(appdata, recoReadNanoCommand)==0){
			PRINTF("Receive [Nano recovery] Command.\n");
			NANOPERIOD = 60;

			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK Start Nano", sizeof("  ACK Start Nano"));
		}
		
		//YIBO: We use three commands to manually control the power mode...
		if(strcmp(appdata, batteryPowerCommand)==0){
			PRINTF("Receive [Battery powered] Command.\n");
			powermode_t = 0;
			appMessage.data.scm_sensors.powermode_yibo = powermode_t;
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK BP", sizeof("  ACK BP"));
		}

		if(strcmp(appdata, solarPanelPowerCommand)==0){
			PRINTF("Receive [solar panel powered] Command.\n");
			powermode_t = 1;
			appMessage.data.scm_sensors.powermode_yibo = powermode_t;
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK SP", sizeof("  ACK SP"));
		}

		if(strcmp(appdata, batteryChargeCommand)==0){
			PRINTF("Receive [battery charging] Command.\n");
			powermode_t = 2;
			appMessage.data.scm_sensors.powermode_yibo = powermode_t;
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK BC", sizeof("  ACK BC"));
		}
		
		//YIBO: collecting frequency modification 10, 15, 30, 60, 120
		if(strcmp(appdata, fm10Command)==0){
			PRINTF("Receive [FM 10] Command.\n");
			change_collecting_frequency(10);
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK FM 10", sizeof("  ACK FM 10"));
		}
		if(strcmp(appdata, fm15Command)==0){
			PRINTF("Receive [FM 15] Command.\n");
			change_collecting_frequency(15);;
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK FM 15", sizeof("  ACK FM 15"));
		}
		if(strcmp(appdata, fm30Command)==0){
			PRINTF("Receive [FM 30] Command.\n");
			change_collecting_frequency(30);;
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK FM 30", sizeof("  ACK FM 30"));
		}
		if(strcmp(appdata, fm60Command)==0){
			PRINTF("Receive [FM 60] Command.\n");
			change_collecting_frequency(60);
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK FM 60", sizeof("  ACK FM 60"));
		}
		if(strcmp(appdata, fm120Command)==0){
			PRINTF("Receive [FM 2 min] Command.\n");
			change_collecting_frequency(120);
			PRINTF("DATA sending reply\n");
    		uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    		uip_udp_packet_send(client_conn, "  ACK FM 2 min", sizeof("  ACK FM 2 min"));
		}

		//YIBO: TX power modification
		next_tx = strchr(appdata, ' ');
		if(next_tx!=NULL){
			PRINTF("next_tx!=NULL \n");
			//strncpy(txpowerCommand_temp, appdata, sizeof(7));
  			//txpowerCommand_temp[7]=0;
  			txpowerCommand_temp[0] = *appdata;
  			txpowerCommand_temp[1] = *(++appdata);
  			txpowerCommand_temp[2] = *(++appdata);
  			txpowerCommand_temp[3] = *(++appdata);
  			txpowerCommand_temp[4] = *(++appdata);
  			txpowerCommand_temp[5] = *(++appdata);
  			txpowerCommand_temp[6] = *(++appdata);
  			txpowerCommand_temp[7] = 0;
			if(strcmp(txpowerCommand_temp, txpowerCommand)==0){
				PRINTF("Receive [txpower] Command.\n");
				++next_tx;
				txpowermodif = (uint8_t)strtolong_yibo(next_tx);
				PRINTF("[txpower] = %u \n", txpowermodif);
				NETSTACK_RDC.off(1);
				rf230_set_new_txpower(txpowermodif);
				NETSTACK_RDC.on();
				
				uip_ipaddr_copy(&client_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    				uip_udp_packet_send(client_conn, "  ACK txpower", sizeof("  ACK txpower"));
			}
		}		

    	//uip_create_unspecified(&client_conn->ripaddr); //YIBO: we still need to send collect-view message
  	}
}
/*---------------------------------------------------------------------------*/
void
collect_common_send(void)
{
  	static uint8_t seqno;
  	struct {
    	uint8_t seqno;
    	uint8_t for_alignment;
    	struct collect_view_data_msg msg;
  	} msg;
  	/* struct collect_neighbor *n; */
  	uint16_t parent_etx;
  	uint16_t rtmetric;
  	uint16_t num_neighbors;
  	uint16_t beacon_interval;
  	rpl_parent_t *preferred_parent;
  	linkaddr_t parent;
  	rpl_dag_t *dag;

  	if(client_conn == NULL) {
    	/* Not setup yet */
    	return;
  	}
  	memset(&msg, 0, sizeof(msg));
  	seqno++;
  	if(seqno == 0) {
    	/* Wrap to 128 to identify restarts */
    	seqno = 128;
  	}
  	msg.seqno = seqno;

  	linkaddr_copy(&parent, &linkaddr_null);
  	parent_etx = 0;

  	/* Let's suppose we have only one instance */
  	dag = rpl_get_any_dag();
  	if(dag != NULL) {
    	preferred_parent = dag->preferred_parent;
    	if(preferred_parent != NULL) {
      		uip_ds6_nbr_t *nbr;
      		nbr = uip_ds6_nbr_lookup(rpl_get_parent_ipaddr(preferred_parent));
      		if(nbr != NULL) {
        		/* Use parts of the IPv6 address as the parent address, in reversed byte order. */
        		parent.u8[LINKADDR_SIZE - 1] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 2];
        		parent.u8[LINKADDR_SIZE - 2] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 1];
        		parent_etx = rpl_get_parent_rank((linkaddr_t *) uip_ds6_nbr_get_ll(nbr)) / 2;
      		}
    	}
    	rtmetric = dag->rank; //YIBO: rtmetric is a big value
    	beacon_interval = (uint16_t) ((2L << dag->instance->dio_intcurrent) / 1000);
		//YIBO: [/1000] means to change mS to S
		//YIBO: (2L << dag->instance->dio_intcurrent) means that trickle timer doubled how many times.
    	num_neighbors = uip_ds6_nbr_num();
  	} else {
    	rtmetric = 0;
    	beacon_interval = 0;
    	num_neighbors = 0;
  	}

  	/* num_neighbors = collect_neighbor_list_num(&tc.neighbor_list); */
  	collect_view_construct_message(&msg.msg, &parent,
                                 	parent_etx, rtmetric,
                                 	num_neighbors, beacon_interval);
	//YIBO:1421601762467 25 2 54530 0 2 174 1 0 17 25411 0 53934 0 469 845 
	//Parent ID; parent_etx; current_rtmetric; num_neighbors; beacon_interval-->[1 128 512 1 2097] 
	//sensors[5] [458 57 45 11 0]

  	uip_udp_packet_sendto(client_conn, &msg, sizeof(msg),
                        	&server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
  	//YIBO: print the powertrace information when a udp packet is sent.
  	powertrace_print("#YB");
  	if(powerleft <= 0){
		PRINTF("Smaller than 0, should Exit this process !!\n");
  	}
}
/*---------------------------------------------------------------------------*/
void
collect_common_net_init(void)
{
/*
#if CONTIKI_TARGET_Z1
  uart0_set_input(serial_line_input_byte);
#else
  uart1_set_input(serial_line_input_byte);
#endif
*/
	rs232_set_input(RS232_PORT_1, serial_line_input_byte);
	serial_line_init();
	
	PRINTF("I am router!\n");
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  /* set server address */
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  set_global_address();

  PRINTF("UDP client process started\n");

  print_local_addresses();
  
  /* The data sink runs with a 100% duty cycle in order to ensure high
    packet reception rates. */
  //NETSTACK_RDC.off(1);

  /* new connection with remote host */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL);
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
        UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
  
  //YIBO: don't know why to put these two lines of code here ?
  powertrace_sniff(POWERTRACE_ON);
  powertrace_print("#YB");

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#if iwotcore_CYB_nano
#include "lib/random.h"
PROCESS_THREAD(sensors_process_nano, ev, data)
{
	static struct etimer et_nano_base, wait_timer;
	
	int32_t nanotick;
	int32_t temp;
	int16_t nanoIOErrNo;
	int16_t nanoIOCountNo;
	uint8_t first4bit;
	uint8_t second4bit;
	uint8_t third4bit;
	uint16_t nanoMetric;
	uint16_t initialNANOPERIOD;
	initialNANOPERIOD = NANOPERIOD;
		
	PROCESS_BEGIN();
	/* Set an etimer. We take sensor readings when it expires and reset it. */
  	etimer_set(&et_nano_base, CLOCK_SECOND * NANOPERIOD);
	while(1) {
		PROCESS_WAIT_EVENT();
		if(ev == PROCESS_EVENT_TIMER) {
			if(data == &et_nano_base) {
				if(initialNANOPERIOD == NANOPERIOD){
					etimer_reset(&et_nano_base);
					etimer_set(&wait_timer, random_rand() % (CLOCK_SECOND * (NANOPERIOD-20))); //YIBO: ensure the NANO is read within 120s
				}else {
					etimer_set(&et_nano_base, NANOPERIOD);
					etimer_set(&wait_timer, random_rand() % (CLOCK_SECOND * (NANOPERIOD-20)));
					initialNANOPERIOD = NANOPERIOD;
				}				
			}
			else if(data == &wait_timer) {
				PRINTF("***Sensors_process_nano Started***\n");
    			collect_common_set_send_active(0);
  				NETSTACK_RDC.off(1);
				
				testAppGetSensorDataNano();
				
				temp = appMessage.data.scm_sensors.NanoRiscStatus;
				nanoIOErrNo = appMessage.data.scm_sensors.NanoIOErrNo;
				nanoIOCountNo = appMessage.data.scm_sensors.NanoIOCountNo;
				PRINTF("NanoIOErrNo = [%x], NanoIOCountNo = [%x]\n", nanoIOErrNo, nanoIOCountNo);
				//YIBO_TODO: nanoIOErrNo needs to be verified, otherwise, some outputs are 0 or illegal.
				//what is the meaning of 0x2020: EXTtoEM_ErrNo_Suc_32Bit (0x2000) | 0x20 = 0x2020 
				if(nanoIOErrNo==0x2020 && nanoIOCountNo==0x20){		
					nanotick = temp << 12;
					nanotick = (nanotick >> 12)&0x000fffff;
					first4bit = (uint8_t)((temp >> 28)&0x0f);
					second4bit = (uint8_t)((temp&0x0f000000)>>24);
					third4bit = (uint8_t)((temp&0x00f00000)>>20);
					PRINTF("tick: [%ld], 1st [%x], 2nd [%x], 3rd [%x]\n", nanotick, first4bit, 
																	second4bit, third4bit);
					nanoMetric = second4bit + third4bit;
					appMessage.data.scm_sensors.NanoMetric = nanoMetric;
					NETSTACK_RDC.on();
					collect_common_set_send_active(1);
					PRINTF("***Sensors_process_nano End***\n");
				}
			}
		}
  	}

  	PROCESS_END();	
}
#endif
/*---------------------------------------------------------------------------*/
