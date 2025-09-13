#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ch.h>
#include <hal.h>
#include <memory_protection.h>
#include <msgbus/messagebus.h>
#include <usbcfg.h>
#include <motors.h>
#include <camera/po8030.h>
#include <chprintf.h>
#include <sensors/proximity.h>
#include <sensors/VL53L0X/VL53L0X.h>

#include <spi_comm.h>
#include "main.h"
#include "color_detection.h"
#include "move.h"
#include "selector.h"
#include "leds.h"
#include "prox_detection.h"

#define STACK_CHK_GUARD 0xe2dee396

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

messagebus_t bus;

static void serial_start(void){
    static SerialConfig ser_cfg = {
        115200,
        0,
        0,
        0,
    };
    sdStart(&SD3, &ser_cfg);
}

int main(void)
{
    halInit();
    chSysInit();
    mpu_init();

    messagebus_init(&bus, &bus_lock, &bus_condvar);

    // starts the serial communication
    serial_start();
    // start the USB communication
    usb_start();
    // starts the camera
    dcmi_start();
    po8030_start();
    // inits the motors
    motors_init();
    //starts the IR sensors
    proximity_start();
    calibrate_ir();
    VL53L0X_start();
    // starts RGB LEDS and User button managment
    spi_comm_start();  

    int selector = 0;
    //Wait for programmer to turn the selector
    while(selector != 1){           
        selector = get_selector();
        chThdSleepMilliseconds(10);
    }
    color_detection_start(&bus);
    move_start(&bus);
    prox_detection_start();
    while(1){
        chThdSleepMilliseconds(20);
    }
}

void __stack_chk_fail(void){
    chSysHalt("Stack smashing detected");
}