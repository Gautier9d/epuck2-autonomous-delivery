#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <sensors/proximity.h>
#include <sensors/VL53L0X/VL53L0X.h>
#include <msgbus/messagebus.h>
#include <stdlib.h>
#include <stdint.h>
#include <usbcfg.h>

#include "prox_detection.h"
#include "main.h"
#include "leds.h"

#define PROXIMITY_THRESHOLD 120                     //Careful to redefine this depending on lighting conditions

static BSEMAPHORE_DECL(wall_sem, TRUE);
static bool wall_nearby = 0;

static THD_WORKING_AREA(waProxDetection, 128); 
static THD_FUNCTION(ProxDetection, arg) {
    
    chRegSetThreadName(__FUNCTION__);
    (void)arg;
    int16_t proximity3 = 0;                      
    int16_t proximity4 = 0;
    while(1){
        proximity3 = get_calibrated_prox(3);     
        proximity4 = get_calibrated_prox(4);
        if((proximity3 + proximity4 > PROXIMITY_THRESHOLD)){ 
            wall_nearby = 1;
            chBSemSignal(&wall_sem);
        }else{
            wall_nearby = 0;
        }
        chThdSleepMilliseconds(50); 
    }
}

void prox_detection_start(void){
    chThdCreateStatic(waProxDetection, sizeof(waProxDetection), NORMALPRIO, ProxDetection, NULL);
}

 /**
 * @brief   Release the semaphore only when a sanity check 
 *          (with the condion) is done, to avoid the release at the wrong time
 *          
 */
void wait_until_wall(void){          
    do{
        chBSemWait(&wall_sem); 
        chThdSleepMilliseconds(5);
    }while(wall_nearby==false);      
}                     
