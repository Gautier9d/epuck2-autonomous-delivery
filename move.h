#ifndef MOVE_H
#define MOVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ch.h>
#include <hal.h>
#include "color_detection.h"  

//List of robot states
typedef enum{
    GO_STRAIGHT,
    DELIVERY,
    RETURN,
    CHANGE_ROW,
    EMERGENCY,
} state_mode_t;

typedef enum{
    RIGHT,
    LEFT,
} direction_t;

void motor_set_position(float position_r, float position_l, float speed_r, float speed_l);
void move_start(messagebus_t *bus);
bool read_color(color_detection_t *color);
void pos_reaching(float step_to_reach, direction_t dir_for_pos);
void motors_turn(float pos_to_reach, direction_t dir_to_turn);
void set_speed_motors(uint16_t speed);

#endif