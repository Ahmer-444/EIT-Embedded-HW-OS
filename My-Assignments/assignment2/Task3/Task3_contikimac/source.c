					/* Task-3 unicast Communication*/

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

I have collected thes results from Mote-4: energies are representated by E_(Name) in the screenshots.

RDC			Transmit Energy(J)	Listen Energy(J)	CPU Energy(J)		LPM Energy(J)
nullrdc 		0.013			20.388			0.089			0.048
xmac			0.027			1.122			0.108			0.047
lpp			0.104			1.360			0.178			0.045
xmac			0.020			1.649			0.283			0.042
contikimac		0.099			0.321			0.138			0.046

Comments: Nullrdc consumes highest energy while contikiMac consumes least with configuration of 8Hz channel rate.

*/

#include "contiki.h"
#include "net/rime.h"
#include "random.h"

//#include "powertrace.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
#include "project-conf.h"

#include "sys/energest.h"

#define VOLT 3.6
#define I_TX 0.0195
#define I_RX 0.0218
#define I_CPU 0.0018
#define I_LPM 0.0000545


/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/

static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  printf("unicast message received from %d.%d\n",
	 from->u8[0], from->u8[1]);
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(example_unicast_process, ev, data)
{
  static uint32_t last_cpu, last_lpm, last_transmit, last_listen;
  uint32_t all_cpu, all_lpm, all_transmit, all_listen;
  uint32_t time, all_time, radio, all_radio;
  uint32_t cpu, lpm, transmit, listen;
  float E_TX,E_RX,E_CPU,E_LPM;
  energest_flush();

  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();

  unicast_open(&uc, 146, &unicast_callbacks);

  while(1) {
    static struct etimer et;
    rimeaddr_t addr;
    
    etimer_set(&et, CLOCK_SECOND);
    
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

printf("transmit = %lu listen = %lu cpu = %lu lpm = %lu\n",transmit,listen,cpu,lpm);
printf("Ticks/sec = %lu\n",CLOCK_SECOND);
printf("E_Transmit = %d.%03u E_Listen = %d.%03u E_CPU = %d.%03u E_LPM = %d.%03u\n",
F1,M1,F2,M2,F3,M3,F4,M4);


    packetbuf_copyfrom("Hello", 5);
    addr.u8[0] = 1;
    addr.u8[1] = 0;
    if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)) {
      unicast_send(&uc, &addr);
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


