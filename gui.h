#ifndef __GUI_H
#define __GUI_H

typedef enum {
    LV_STATUS_BAR_BATTERY_LEVEL = 0,
} lv_icon_status_bar_t;

void setupGui();
void updateStepCounter(int counter);
void updateBatteryLevel();

#endif /*__GUI_H */