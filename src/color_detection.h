#ifndef COLOR_DETECTION_H
#define COLOR_DETECTION_H

#define USED_LINE 200   // Must be inside [0..478]

//List of colors detected
typedef enum {
    RED_COLOR,
    GREEN_COLOR,
    BLUE_COLOR,
    BLACK_COLOR,
    WHITE_COLOR,
} color_detection_t;

uint32_t extract_color_mean(uint8_t *buffer);
void find_dominant_color(uint32_t red, uint32_t blue, uint32_t green);
void color_detection_start(messagebus_t *bus);
void publish_color(color_detection_t color);

#endif /* PROCESS_IMAGE_H */
