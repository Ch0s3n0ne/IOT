/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 */

/**
 * \file
 *      DEEC CoAP Example Client
 * \author
 *      Paulo Peixoto
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "random.h"
#include "er-coap-engine.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
/* These library allows to use the on-board sensors */
#if CONTIKI_TARGET_ZOUL
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h"
#else /* Assumes the Z1 mote */
#include "dev/tmp102.h"
#include "dev/adxl345.h"
#include "dev/battery-sensor.h"
#endif


#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

#define UDP_SERVER_PORT 5683
#define UDP_CLIENT_PORT 5683

#define EXAMPLE_TX_POWER  31
#define EXAMPLE_CHANNEL   26

#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xfd00, 0, 0, 0, 0, 0, 0, 0x1) // Address of VM on TunSlip6 - fd00::1 

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

#define TOGGLE_INTERVAL 15

#define COAP_RESPONSE_OK             69
#define COAP_RESPONSE_NOT_FOUND     132

static int deviceID=2032;

PROCESS(er_example_client, "DEEC CoAP Example Client");
AUTOSTART_PROCESSES(&er_example_client);

uip_ipaddr_t server_ipaddr;
static struct etimer et;
static coap_packet_t request[1]; // This way the packet can be treated as pointer as usual.

/*----------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("CoAP Client IPv6 addresses:\n");
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


/*----------------------------------------------------------------------------*/
static void
print_radio_values(void)
{
  radio_value_t aux;

  printf("\n* Client Radio parameters:\n");

  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &aux);
  printf("   Channel %u", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, &aux);
  printf(" (Min: %u, ", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, &aux);
  printf("Max: %u)\n", aux);

  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &aux);
  printf("   Tx Power %3d dBm", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_TXPOWER_MIN, &aux);
  printf(" (Min: %3d dBm, ", aux);

  NETSTACK_RADIO.get_value(RADIO_CONST_TXPOWER_MAX, &aux);
  printf("Max: %3d dBm)\n", aux);

  NETSTACK_RADIO.get_value(RADIO_PARAM_PAN_ID, &aux);
  printf("   PAN ID: %04X\n", aux);
}

/*----------------------------------------------------------------------------*/
/* Callback to handle server response. */
void client_chunk_handler(void *response)
{
	const uint8_t *chunk;

	int len = coap_get_payload(response, &chunk);
	int returnCode = ((coap_packet_t  *)response)->code;

	printf("Reply Received! Code: %d\n",returnCode);
	if(returnCode==COAP_RESPONSE_NOT_FOUND){
		printf("Service Not Found!\n\n");
	}else{
		if(returnCode==COAP_RESPONSE_OK){
			printf("Res:%s (%d bytes)\n\n", (char *)chunk,len);
		}else{
			printf("Server Error!\n\n");
		}
	}
}



/*----------------------------------------------------------------------------*/
void sendAddRequest(){
	uint16_t firstNum  = random_rand() % 20+20; // Random Number Between 0 - 39
	uint16_t secondNum = random_rand() % 20+20; // Random Number Between 0 - 39

	// send a Add request to CoAP Server
	static char fullUrl[100];	
	sprintf(fullUrl,"addNums/%d/%d",firstNum,secondNum); // URL: /addNums/num1/num2 (GET)

	printf("\n--Requesting %s--\n", fullUrl);

	coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
	coap_set_header_uri_path(request, fullUrl);
}

/*---------------------------------------------------------------------------*/
static void
set_radio_default_parameters(void)
{
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, EXAMPLE_TX_POWER);
  printf("   SETTING PAN ID: %04X\n", IEEE802154_CONF_PANID);
  NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, IEEE802154_CONF_PANID);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, EXAMPLE_CHANNEL);
}


/*----------------------------------------------------------------------------*/
PROCESS_THREAD(er_example_client, ev, data)
{
	PROCESS_BEGIN();

	// Initialize Sensors
	#if CONTIKI_TARGET_ZOUL
	  /*  The sensors are already started at boot */
	#else
	  static int8_t x_axis;
	  static int8_t y_axis;
	  static int8_t z_axis;

	  /* Initialize the sensors, the SENSORS_ACTIVATE(...) macro invokes the
	   * configure(...) method of Contiki's sensor API
	   */
	  SENSORS_ACTIVATE(adxl345);
	  SENSORS_ACTIVATE(tmp102);
	  SENSORS_ACTIVATE(battery_sensor);
	#endif
	SENSORS_ACTIVATE(button_sensor); // For both Devices

	// Initialize CoAP API
	SERVER_NODE(&server_ipaddr);

	printf("Server Address: ");
	PRINT6ADDR(&server_ipaddr);
	PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

	printf("Local Address: ");
	print_local_addresses();

	/* receives all CoAP messages - Deal With CoAP Responses */
	coap_init_engine();

	etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);

	set_radio_default_parameters();

	print_radio_values();

	printf("\nAdd Request Will Be Issued Every %d Seconds!\n\n",TOGGLE_INTERVAL);

	while(1) {
		PROCESS_YIELD();

		if(etimer_expired(&et)) {
			sendAddRequest(); // Prepare CoAP Packet Buffer
			COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,client_chunk_handler); // Send CoAP Packet

			etimer_reset(&et);
		}
	}

	PROCESS_END();
}
