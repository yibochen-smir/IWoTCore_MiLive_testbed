/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
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
 * This file is part of the Configurable Sensor Network Application
 * Architecture for sensor nodes running the Contiki operating system.
 *
 * This is a dummy non-functional dummy implementation.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne, Simon Barner
 * Created : 2005-11-03
 * Updated : $Date: 2006/12/22 17:05:31 $
 *           $Revision: 1.1 $
 */

#include "contiki-conf.h"
#include "dev/leds.h"
#include <avr/io.h>



#if 1
#define halInitFirstLed() 	  (DDRE |= (1 << 2), PORTE |= (1 << 2))
#define halUnInitFirstLed()	  (DDRE &= ~(1 << 2), PORTE &= ~(1 << 2))
#define halOnFirstLed()		  (PORTE &= ~(1 << 2))
#define halOffFirstLed()	  (PORTE |= (1 << 2))
#define halToggleFirstLed()	  (PORTE ^= (1 << 2))
  
  
#define halInitSecondLed()		(DDRE |= (1 << 3), PORTE |= (1 << 3))
#define halUnInitSecondLed()	(DDRE &= ~(1 << 3), PORTE &= ~(1 << 3))
#define halOnSecondLed()		(PORTE &= ~(1 << 3))
#define halOffSecondLed()		(PORTE |= (1 << 3))
#define halToggleSecondLed()	(PORTE ^= (1 << 3))
  
#define halInitThirdLed()		(DDRE |= (1 << 4), PORTE |= (1 << 4))
#define halUnInitThirdLed() 	(DDRE &= ~(1 << 4), PORTE &= ~(1 << 4))
#define halOnThirdLed() 		(PORTE &= ~(1 << 4))
#define halOffThirdLed()		(PORTE |= (1 << 4))
#define halToggleThirdLed() 	(PORTE ^= (1 << 4))

#endif

#if 0

 /*---------------------------------------------------------------------------*/
 void
 leds_arch_init(void)
 {
	 halInitFourthLed();
 }
 /*---------------------------------------------------------------------------*/
 unsigned char
 leds_arch_get(void)
 {
	 return 0;
 }
 /*---------------------------------------------------------------------------*/
 void
 leds_arch_set(unsigned char leds)
 {
	 switch (leds)
	 {
		 case LEDS_RED:
		   halOnFourthLed();
		   break;
	 }
 }
 /*---------------------------------------------------------------------------*/
#endif
#if 1
 /* LED ports for RCB*/
#define LEDS_PxDIR DDRE // port direction register
#define LEDS_PxOUT PORTE // port register

 
#define LEDS_CONF_GREEN  (1 << 2) // green led
#define LEDS_CONF_YELLOW (1 << 3) // yellow led
#define LEDS_CONF_RED    (1 << 4) //red led
 
 
 /*---------------------------------------------------------------------------*/
 void leds_arch_init(void)
 {
   LEDS_PxDIR |= (LEDS_CONF_GREEN | LEDS_CONF_YELLOW | LEDS_CONF_RED);
   LEDS_PxOUT |= (LEDS_CONF_GREEN | LEDS_CONF_YELLOW | LEDS_CONF_RED);
 }
 /*---------------------------------------------------------------------------*/
 unsigned char leds_arch_get(void)
 {
   return ((LEDS_PxOUT & LEDS_CONF_GREEN) ? 0 : LEDS_GREEN)
	 | ((LEDS_PxOUT & LEDS_CONF_YELLOW) ? 0 : LEDS_YELLOW)
	 | ((LEDS_PxOUT & LEDS_CONF_RED) ? 0 : LEDS_RED);
 }
 /*---------------------------------------------------------------------------*/
 void leds_arch_set(unsigned char leds)
 {
   LEDS_PxOUT = (LEDS_PxOUT & ~(LEDS_CONF_GREEN|LEDS_CONF_YELLOW|LEDS_CONF_RED))
	 | ((leds & LEDS_GREEN) ? 0 : LEDS_CONF_GREEN)
	 | ((leds & LEDS_YELLOW) ? 0 : LEDS_CONF_YELLOW)
	 | ((leds & LEDS_RED) ? 0 : LEDS_CONF_RED);
 }
 /*---------------------------------------------------------------------------*/
#endif

#if 0
 /* LED ports for IWoTCore*/
#define LEDS_PxDIR DDRD // port direction register
#define LEDS_PxOUT PORTD // port register

#define LEDS_CONF_GREEN    (1 << 0) //green led
#define LEDS_CONF_YELLOW (1 << 1) // yellow led

 
 
 /*---------------------------------------------------------------------------*/
 void leds_arch_init(void)
 {
   LEDS_PxDIR |= (LEDS_CONF_GREEN | LEDS_CONF_YELLOW);
   LEDS_PxOUT |= (LEDS_CONF_GREEN | LEDS_CONF_YELLOW);
 }
 /*---------------------------------------------------------------------------*/
 unsigned char leds_arch_get(void)
 {
   return ((LEDS_PxOUT & LEDS_CONF_GREEN) ? 0 : LEDS_GREEN)
	 | ((LEDS_PxOUT & LEDS_CONF_YELLOW) ? 0 : LEDS_YELLOW);
 }
 /*---------------------------------------------------------------------------*/
 void leds_arch_set(unsigned char leds)
 {
   LEDS_PxOUT = (LEDS_PxOUT & ~(LEDS_CONF_GREEN|LEDS_CONF_YELLOW))
	 | ((leds & LEDS_GREEN) ? 0 : LEDS_CONF_GREEN)
	 | ((leds & LEDS_YELLOW) ? 0 : LEDS_CONF_YELLOW);
 }
 /*---------------------------------------------------------------------------*/
#endif

#if 0
 /* LED ports for IWoTCore*/
#define LEDS_PxDIR DDRD // port direction register
#define LEDS_PxOUT PORTD // port register

#define LEDS_CONF_RED    (1 << 4) //red led

 
 /*---------------------------------------------------------------------------*/
 void leds_arch_init(void)
 {
   LEDS_PxDIR |= (LEDS_CONF_RED);
   LEDS_PxOUT |= (LEDS_CONF_RED);
 }
 /*---------------------------------------------------------------------------*/
 unsigned char leds_arch_get(void)
 {
   return ((LEDS_PxOUT & LEDS_CONF_RED) ? 0 : LEDS_RED);
 }
 /*---------------------------------------------------------------------------*/
 void leds_arch_set(unsigned char leds)
 {
	//{PORT##port &= ~(1 << bit);}
   LEDS_PxOUT = (LEDS_PxOUT & ~(LEDS_CONF_RED))
	 | ((leds & LEDS_RED) ? 0 : LEDS_CONF_RED);
 }
 /*---------------------------------------------------------------------------*/
#endif

#if 0
 /* LED ports for IWoTCore*/
#define LEDS_PxDIR DDRG // port direction register
#define LEDS_PxOUT PORTG // port register

#define LEDS_CONF_YELLOW (1 << 1) // yellow led

 
 /*---------------------------------------------------------------------------*/
 void leds_arch_init(void)
 {
   LEDS_PxDIR |= (LEDS_CONF_YELLOW);
   LEDS_PxOUT |= (LEDS_CONF_YELLOW);
 }
 /*---------------------------------------------------------------------------*/
 unsigned char leds_arch_get(void)
 {
   return ((LEDS_PxOUT & LEDS_CONF_YELLOW) ? 0 : LEDS_YELLOW);
 }
 /*---------------------------------------------------------------------------*/
 void leds_arch_set(unsigned char leds)
 {
   LEDS_PxOUT = (LEDS_PxOUT & ~(LEDS_CONF_YELLOW))
	 | ((leds & LEDS_YELLOW) ? 0 : LEDS_CONF_YELLOW);
 }
 /*---------------------------------------------------------------------------*/
#endif



