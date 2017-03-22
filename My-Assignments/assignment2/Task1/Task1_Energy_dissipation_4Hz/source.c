					/* Task-1 */

/* 

From Dataheet (Sky-Mote)
Voltage		: 	3.6V
I_TX		:	19.5 mA
I_RX		:	21.8 mA
I_CPU		:	1800 uA
I_LPM		:	54.5 uA

Farmulae: (No. of Ticks * V * I) / Ticks_Per_Sec
Ticks_Per_Sec = CLOCK_SECOND = 128 ticks

No. of ticks are shown in the simulation images. (in current directory)

Channel Rate(HZ)	Transmit Energy(J)	Listen Energy(J)	CPU Energy(J)		LPM Energy(J)
4 			2.35			0.529			0.756			0.236
8			1.186			0.908			0.549			0.243
16			0.603			1.914			0.513			0.306

Comments: Listen energy increases with the increase in Channel Rate, while all other decreases.

*/
#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
#include <math.h>
#include "sys/energest.h"
#include "project-conf.h"

#define VOLT 3.6
#define I_TX 0.0195
#define I_RX 0.0218
#define I_CPU 0.0018
#define I_LPM 0.0000545

 
/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "BROADCAST example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;
  static uint32_t last_cpu, last_lpm, last_transmit, last_listen;
  uint32_t all_cpu, all_lpm, all_transmit, all_listen;
  uint32_t time, all_time, radio, all_radio;
  uint32_t cpu, lpm, transmit, listen;
  float E_TX,E_RX,E_CPU,E_LPM;
  energest_flush();

  //static unsigned long rx_start_time,lpm_start_time,cpu_start_time,tx_start_time;
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();


  /* Start powertracing, once every two seconds. */
  //powertrace_start(CLOCK_SECOND * 2);
  
  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

cpu = all_cpu - last_cpu;
lpm = all_lpm - last_lpm;
transmit = all_transmit - last_transmit;
listen = all_listen - last_listen;

last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

radio = transmit + listen;
time = cpu + lpm;
all_time = all_cpu + all_lpm;
all_radio = energest_type_time(ENERGEST_TYPE_LISTEN) + energest_type_time(ENERGEST_TYPE_TRANSMIT);

E_TX = (I_TX * VOLT) / CLOCK_SECOND;
unsigned int X1 = (E_TX - floor(E_TX))*1000 * transmit;
int F1 = X1/1000;
int M1 = X1%1000; 

E_RX = (I_RX * VOLT) / CLOCK_SECOND;
unsigned int X2 = (E_RX - floor(E_RX))*1000 * listen;
int F2 = X2/1000;
int M2 = X2%1000; 


E_CPU = (I_CPU * VOLT) / CLOCK_SECOND;
unsigned int X3 = (E_CPU - floor(E_CPU))*1000 * cpu;
int F3 = X3/1000;
int M3 = X3%1000; 

E_LPM = (I_LPM * VOLT) / CLOCK_SECOND;
unsigned int X4 = (E_LPM - floor(E_LPM))*1000 * lpm;
int F4 = X4/1000;
int M4 = X4%1000; 

printf("E_Transmit = %d.%03u E_Listen = %d.%03u E_CPU = %d.%03u E_LPM = %d.%03u\n",
F1,M1,F2,M2,F3,M3,F4,M4);

    printf("broadcast message sent\n");
    printf("transmit = %lu listen = %lu cpu = %lu lpm = %lu\n",transmit,listen,cpu,lpm);
    printf("Ticks/sec = %lu\n",CLOCK_SECOND);

    packetbuf_copyfrom("Hello", 6);
    broadcast_send(&broadcast);
    printf("broadcast message sent\n");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

