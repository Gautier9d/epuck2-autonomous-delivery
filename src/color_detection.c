#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <usbcfg.h>
#include <camera/po8030.h>
#include <msgbus/messagebus.h>
#include <stdbool.h>
#include <leds.h>
#include <chprintf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "color_detection.h"
#include "move.h"
#include "main.h"
#include "selector.h"

#define IMAGE_BUFFER_SIZE   640
#define BLACK_THRESHOLD     200     //Careful to redefine this depending on lighting conditions
#define WHITE_THRESHOLD     180     //Careful to redefine this depending on lighting conditions

static messagebus_topic_t color_topic;
static uint8_t the_color;

 /**
 * @brief   Perform the average intensity, out of 255 of the pixels 
 *          in the image taken
 *
 * @param[in] buffer       pointer to a table of intensities of each pixel
 *
 */
uint32_t extract_color_mean(uint8_t *buffer){   
    uint32_t mean = 0;
    for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
        mean += buffer[i];
    }
    mean /= IMAGE_BUFFER_SIZE;
    return mean;
}

static THD_WORKING_AREA(waColorDetection, 1024);
static THD_FUNCTION(ColorDetection, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
    po8030_advanced_config(FORMAT_RGB565, 0, USED_LINE, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1); 
    dcmi_enable_double_buffering();
    dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
    dcmi_prepare();
    po8030_set_awb(0);

    MUTEX_DECL(color_topic_lock);
    CONDVAR_DECL(color_topic_condvar);
    messagebus_topic_init(&color_topic, &color_topic_lock, &color_topic_condvar, &the_color, sizeof(the_color));
    messagebus_advertise_topic(&bus, &color_topic, "color_topic");

    uint8_t *img_buff_ptr;
    uint8_t image[IMAGE_BUFFER_SIZE] = {0};
    uint32_t red_mean = 0;   
    uint32_t blue_mean = 0;  
    uint32_t green_mean = 0;   

    while(1){
        dcmi_capture_start();                                                 //starts a capture
        wait_image_ready();                                                   //waits for the capture to be done
        img_buff_ptr = dcmi_get_last_image_ptr();

        for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){             //RED Extraction
            image[i/2] = (uint8_t)img_buff_ptr[i] & 0xF8;
        }
        red_mean = extract_color_mean(image);

        for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){             //BLUE Extraction
            image[i/2] = ((uint8_t)img_buff_ptr[i+1] & 0x1F) << 3;
        }
        blue_mean = extract_color_mean(image);

        for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){             //GREEN Extraction
            image[i/2] = (((uint8_t)img_buff_ptr[i] & 0x07) << 5 )+ (((uint8_t)img_buff_ptr[i+1] & 0xE0) >> 3);
        }
        green_mean = extract_color_mean(image);

        find_dominant_color(red_mean, blue_mean, green_mean);
        publish_color(the_color);
        chThdSleepMilliseconds(100);                                          //No need to use every single image
    }
}

 /**
 * @brief   Compare the different color mean to identify the dominant color   
 *          of the taken image
 *         
 * @param red        red mean intensity 
 * @param blue       blue mean intensity
 * @param green      green mean intensity
 */
void find_dominant_color(uint32_t red, uint32_t blue, uint32_t green){
    if((red > WHITE_THRESHOLD) && (green > WHITE_THRESHOLD) && (blue > WHITE_THRESHOLD)){
        the_color = WHITE_COLOR;
    }else if((red + blue + green) < BLACK_THRESHOLD){
        the_color = BLACK_COLOR;
    }else if((red > green) && (red > blue)){      
        the_color = RED_COLOR;                                                 
    }else if(blue > (green+10)){
        the_color = BLUE_COLOR;
    }else{
        the_color = GREEN_COLOR;
    }
}

void color_detection_start(messagebus_t *bus){
    chThdCreateStatic(waColorDetection, sizeof(waColorDetection), NORMALPRIO, ColorDetection, bus);
}

void publish_color(color_detection_t color){
    messagebus_topic_publish(&color_topic, &color, sizeof(color));
}
