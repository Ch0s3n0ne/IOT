/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"

#include "lib/sensors.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include "dev/temperature-sensor.h"

#include "sys/etimer.h"
/*---------------------------------------------------------------------------*/
PROCESS(Main_process, "Led control");
PROCESS(user_button_interaction, "User Button");

AUTOSTART_PROCESSES(&Main_process,&user_button_interaction);
/*---------------------------------------------------------------------------*/
static struct etimer et;
static uint16_t state = 0;
static uint16_t i=0;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(Main_process, ev, data)
{
    
  PROCESS_BEGIN();
    static uint16_t temp;

    SENSORS_ACTIVATE(temperature_sensor);

    SENSORS_ACTIVATE(button_sensor);

    while(1) {


        if (state==0)       
        {  
            printf("Pronto a começar \n"); 
            leds_on(LEDS_ALL);         
            PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
            
        }     
        else if(state==1)
        {
            
            etimer_set(&et, CLOCK_SECOND);

            temp = temperature_sensor.value(0);

            printf("Temperature: %dC\n", temp ); 

            if(temp<=33){

                leds_on(LEDS_BLUE);
                leds_off(LEDS_RED);
                leds_off(LEDS_GREEN);
            }
            else if(temp<=67 && temp>=34){

                leds_on(LEDS_GREEN);
                leds_off(LEDS_RED);
                leds_off(LEDS_BLUE);

            }
            else{

                leds_on(LEDS_RED);
                leds_off(LEDS_GREEN);
                leds_off(LEDS_BLUE);
            }   

            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

            etimer_reset(&et);
        }
        else{
                        
            etimer_set(&et, CLOCK_SECOND);

            if(i==0){

                leds_on(LEDS_BLUE);
                leds_off(LEDS_RED);
                leds_off(LEDS_GREEN);
                i=1;
            }
            else if(i==1){

                leds_on(LEDS_GREEN);
                leds_off(LEDS_RED);
                leds_off(LEDS_BLUE);
                i=2;

            }
            else{

                leds_on(LEDS_RED);
                leds_off(LEDS_GREEN);
                leds_off(LEDS_BLUE);
                i=0;
            }   

            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

            etimer_reset(&et);
           
        } 
    }

  PROCESS_END();
}

PROCESS_THREAD(user_button_interaction, ev, data)
{
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);

    printf("mudou de estado \n");

    leds_off(LEDS_ALL);

    if (state==1)
    {
        state=2;
    }
    else
    {
        state=1;
    }

    
    
        

  }

  PROCESS_END();
}
