#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <msgbus/messagebus.h>
#include <leds.h>

#include "main.h"
#include "motors.h"
#include "move.h"
#include "color_detection.h"
#include "prox_detection.h"

#define PI                  3.1415926536f
#define NSTEP_ONE_TURN      1000  // number of step for 1 turn of the motor
#define WHEEL_PERIMETER     13.0  // [cm] Carefull to have .0 so that the division is done in decimal for STEP_TO_90 et STEP_TO_180
#define WHEEL_DISTANCE      5.35f //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)
#define QUARTER_PERIMETER   (PERIMETER_EPUCK/4.0)
#define HALF_PERIMETER      (PERIMETER_EPUCK/2.0)
#define STEP_TO_90          (QUARTER_PERIMETER * NSTEP_ONE_TURN / WHEEL_PERIMETER)
#define STEP_TO_180         (HALF_PERIMETER * NSTEP_ONE_TURN / WHEEL_PERIMETER)
#define SPEED_RUN           500
#define STOP                0

static state_mode_t run_mode = GO_STRAIGHT;
static color_detection_t color_received;      
static bool turning = 1;                            
static bool emergency = 1;
static messagebus_topic_t *color_topic;

static BSEMAPHORE_DECL(pos_reached_sem, TRUE);

static THD_WORKING_AREA(waMovement, 512);
static THD_FUNCTION(Movement, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    color_topic = messagebus_find_topic_blocking(&bus, "color_topic");

    while(1){
        switch(run_mode){
            case GO_STRAIGHT:
                set_speed_motors(SPEED_RUN); 
                if(read_color(&color_received)){
                    if(color_received == RED_COLOR){    
                        run_mode = DELIVERY;
                    }else if(color_received == GREEN_COLOR){
                        run_mode = CHANGE_ROW;  
                    }else if(color_received == WHITE_COLOR){
                        run_mode = EMERGENCY;
                    }
                }
                break;
            
            case DELIVERY: 
                chThdSleepMilliseconds(750);             //Time needed by the robot to cross enough the red line in order to avoid re-detecting it when coming back
                motors_turn(STEP_TO_90, RIGHT);
                set_speed_motors(SPEED_RUN);     
                wait_until_wall();
                motors_turn(STEP_TO_180, LEFT);
                run_mode = RETURN;
                break;

            case RETURN: 
                set_speed_motors(SPEED_RUN);
                if(read_color(&color_received)){
                    if(color_received == BLUE_COLOR){
                        chThdSleepMilliseconds(250);      //Time needed by the robot to step forward enough after the blue line to be realigned
                        motors_turn(STEP_TO_90, RIGHT);
                        run_mode = GO_STRAIGHT;
                    }
                }
                break;

            case CHANGE_ROW:  
                motors_turn(STEP_TO_90, LEFT);
                set_speed_motors(SPEED_RUN);
                chThdSleepSeconds(5);
                motors_turn(STEP_TO_90, LEFT);
                run_mode = GO_STRAIGHT;
                break;

            case EMERGENCY:  
                if(emergency){
                    motors_turn(STEP_TO_90, RIGHT);
                    set_speed_motors(SPEED_RUN); 
                    emergency = 0;
                }
                wait_until_wall();
                motors_turn(STEP_TO_90, LEFT);
                set_speed_motors(STOP); 
                break;

            default:
                break;
        }
        chThdSleepMilliseconds(50);
    }
}

void move_start(messagebus_t *bus){
    chThdCreateStatic(waMovement, sizeof(waMovement), NORMALPRIO + 1, Movement, bus);
}

bool read_color(color_detection_t *color){
    if(color_topic != NULL) {
        return messagebus_topic_read(color_topic, color, sizeof(*color));
    }
    return false;
}

 /**
 * @brief   Check if the motors have reached the number of steps requested 
 *          
 * @param step_to_reach     number of step to reach
 * @param dir_for_pos       sense of rotation
 */
void pos_reaching(float step_to_reach, direction_t dir_for_pos){   
    int32_t nb_step_right = right_motor_get_pos(); 
    int32_t nb_step_left = left_motor_get_pos(); 

    if(dir_for_pos == RIGHT){
        if ((nb_step_left >= step_to_reach) && (nb_step_right <= -step_to_reach)){
            chBSemSignal(&pos_reached_sem);
            turning = 0;
        }
     }else if(dir_for_pos == LEFT){
        if((nb_step_left <= -step_to_reach) && (nb_step_right >= step_to_reach)){
            chBSemSignal(&pos_reached_sem);
            turning = 0;
        }
    }
}

 /**
 * @brief   Rotates the robot to the left or right with a certain number of steps 
 *          
 * @param pos_to_reach      number of step to perform
 * @param dir_to_turn       sense of rotation
 */
void motors_turn(float pos_to_reach, direction_t dir_to_turn){
    if(dir_to_turn == RIGHT){
        left_motor_set_speed(500);
        right_motor_set_speed(-500);
    }else if(dir_to_turn == LEFT){
        left_motor_set_speed(-500);
        right_motor_set_speed(500);
    }
    left_motor_set_pos(0);
    right_motor_set_pos(0);
    do{
        pos_reaching(pos_to_reach, dir_to_turn);
        chThdSleepMicroseconds(100);
    }while(turning);
    chBSemWait(&pos_reached_sem); 
    set_speed_motors(STOP);
    turning = 1;
}

void set_speed_motors(uint16_t speed){
    left_motor_set_speed(-speed);        
    right_motor_set_speed(-speed);
}