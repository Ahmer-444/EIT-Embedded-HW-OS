						/* Task-3 Simulation Observation */
/*
Comments:
The simulation screenshots in the current directory depicts that radio makes more number of tries to tranmit/recieve the sensor's data (button) by increasing etimer values. tries : 10 power(-20) * CLOCK_SECOND < 10 power(-14) * CLOCK_SECOND < 10 power(-6) * CLOCK_SECOND
*/

#include "contiki.h"
#include "net/rime/trickle.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
#include <math.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_trickle_process, "Trickle example");
AUTOSTART_PROCESSES(&example_trickle_process);
/*---------------------------------------------------------------------------*/
static void
trickle_recv(struct trickle_conn *c)
{
  printf("%d.%d: trickle message received '%s'\n",
	 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	 (char *)packetbuf_dataptr());
}
const static struct trickle_callbacks trickle_call = {trickle_recv};
static struct trickle_conn trickle;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_trickle_process, ev, data)
{
  PROCESS_EXITHANDLER(trickle_close(&trickle);)
  PROCESS_BEGIN();


  trickle_open(&trickle, 10^(-6)*CLOCK_SECOND, 145, &trickle_call);
  SENSORS_ACTIVATE(button_sensor);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			     data == &button_sensor);

    packetbuf_copyfrom("Hello, world", 13);
    trickle_send(&trickle);

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
