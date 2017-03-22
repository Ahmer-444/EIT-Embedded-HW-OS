						/* Task-1 Simulation Observation */
/*
The simulation screenshot in the current directory (task1_screenshoot.jpg) shows that we have a rime network of 4 motes. Each mote is broadcasting a message to all the nodes whether an event is either triggered by "timer (2-4 secs)" or by a button event.
*/


#include "contiki.h"
#include "net/rime/rime.h"
#include "net/rime/trickle.h"

#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(task1_broadcast_process, "Broadcast task with timer and button event");
AUTOSTART_PROCESSES(&task1_broadcast_process);
/*---------------------------------------------------------------------------*/

static void trickle_recv(struct trickle_conn *c)
{
  printf("%d.%d: trickle message received '%s'\n",
	 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	 (char *)packetbuf_dataptr());
}

const static struct trickle_callbacks trickle_call = {trickle_recv};
static struct trickle_conn trickle;

static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(task1_broadcast_process, ev, data)
{
  static struct etimer et;
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
  PROCESS_EXITHANDLER(trickle_close(&trickle);)

  PROCESS_BEGIN();


  broadcast_open(&broadcast, 129, &broadcast_call);
  trickle_open(&trickle, CLOCK_SECOND, 145, &trickle_call);
  SENSORS_ACTIVATE(button_sensor);


  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 2 + random_rand() % (CLOCK_SECOND * 2));

    PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&et) || (ev == sensors_event &&
			     data == &button_sensor) );

    if (ev == sensors_event)
    {
	//printf("button pressed\n");
	packetbuf_copyfrom("button pressed", 15);
    	trickle_send(&trickle);

    }
    else
    {
	//printf("Timer Event Ocuured\n");
	packetbuf_copyfrom("Timer Event", 12);
	broadcast_send(&broadcast);
    	printf("broadcast message sent\n");

    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

