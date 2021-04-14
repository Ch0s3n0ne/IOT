/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"
#include "lib/sensors.h"
#include "dev/z1-phidgets.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include "sys/ctimer.h"
#include "sys/rtimer.h"
#include "sys/etimer.h"
#include "sys/stimer.h"
/*---------------------------------------------------------------------------*/
PROCESS(test_adc_process, "Test ADC");
PROCESS(user_button_interaction,"User Button");

AUTOSTART_PROCESSES(&test_adc_process, &user_button_interaction);
/*---------------------------------------------------------------------------*/
static struct etimer et;
static struct etimer et2;

static uint16_t indicator = 0;
static uint16_t running = 0;
static uint16_t spin_once = 1;


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_adc_process, ev, data)
{
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(phidgets);

  /* Connect an analogue sensor and measure its value! */

  etimer_set(&et, CLOCK_SECOND*4);

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if(running==0 && spin_once==1){
       leds_off(LEDS_BLUE);
       spin_once=0;
    }
    if(indicator==1)
    {
      leds_on(LEDS_ALL);
      indicator=0;
      spin_once=1;
      printf("indicador \n");
    }
    else if (phidgets.value(PHIDGET3V_1)>127)
    {
      leds_on(LEDS_RED);
      leds_off(LEDS_GREEN);
    }
    else{
      leds_on(LEDS_GREEN);
      leds_off(LEDS_RED);
    }


    printf("ADC1(3V) :%d\n", phidgets.value(PHIDGET3V_1));


    etimer_reset(&et);
  }

  PROCESS_END();
}

PROCESS_THREAD(user_button_interaction, ev, data)
{
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);

  while(1) {

  PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);


  leds_on(LEDS_BLUE);

  running=1;

  etimer_set(&et2,CLOCK_SECOND*4);

  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et2));
  
  leds_off(LEDS_BLUE);

  running=0;
  indicator = 1;

  etimer_reset(&et2);


  }

  PROCESS_END();
}
