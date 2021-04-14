/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"

#include "lib/sensors.h"

#include "dev/leds.h"
#include "dev/leds.h"

#include "dev/button-sensor.h"

#include "dev/temperature-sensor.h"

#include "sys/etimer.h"
/*---------------------------------------------------------------------------*/
PROCESS(Main_process, "Temperatura simulada");


AUTOSTART_PROCESSES(&Main_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
static int16_t temp;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(Main_process, ev, data)
{
    PROCESS_BEGIN();
   
    SENSORS_ACTIVATE(temperature_sensor);

    while(1) {

        temp = temperature_sensor.value(0);

        printf("Valor de Temperatura %d \n", temp);

        etimer_set(&et, CLOCK_SECOND*1);
        
        etimer_reset(&et);
        

    }

  PROCESS_END();
}


