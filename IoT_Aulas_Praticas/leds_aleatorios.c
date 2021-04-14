/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"
#include "lib/sensors.h"
#include "dev/z1-phidgets.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include "dev/battery-sensor.h"

#include "dev/tmp102.h"

#include "sys/ctimer.h"
#include "sys/rtimer.h"
#include "sys/etimer.h"
#include "sys/stimer.h"
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
    static uint32_t batt;

    SENSORS_ACTIVATE(tmp102);
    SENSORS_ACTIVATE(battery_sensor);

    SENSORS_ACTIVATE(button_sensor);

    while(1) {



        if (state==0)       
        {   
            leds_on(LEDS_ALL);         
            printf("todos abertos \n"); 
            PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
            
        }     
        else if(state==1)
        {
            
            etimer_set(&et, CLOCK_SECOND*5);

            if(i==0){
               temp   = tmp102.value(TMP102_READ);
               printf("Temperature: %d.%u C\n", temp / 100, temp % 100); 
               i=1;
               leds_on(LEDS_BLUE);
               leds_off(LEDS_GREEN);
            }
            else{
                batt   = battery_sensor.value(1);

                batt *= 5000;
                batt /= 4095;
                printf("Battery: %u\n\n", (uint16_t)batt);
                i=0;
                leds_on(LEDS_GREEN);
                leds_off(LEDS_BLUE);

            }
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

            etimer_reset(&et);
        }
        else{
  
            temp   = tmp102.value(TMP102_READ);
            printf("Temperature: %d.%u C\n", temp / 100, temp % 100); 
            i=1;
            

            batt   = battery_sensor.value(1);

            batt *= 5000;
            batt /= 4095;
            printf("Battery: %u\n\n", (uint16_t)batt);


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

    leds_off(LEDS_ALL);

    if(state==2)
    {
        state=0;
    }
    else{
        state=state+1;
        printf("estado subiu \n" );
        }
        

  }

  PROCESS_END();
}
