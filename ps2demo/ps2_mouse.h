// ps2_mouse.h
#ifndef PS2_MOUSE_H
#define PS2_MOUSE_H

void ps2_mouse_init(uint8_t clk_pin, uint8_t data_pin);
void ps2_mouse_task(void);
void ps2_mouse_send_packet(int8_t x, int8_t y, uint8_t buttons);

#endif // PS2_MOUSE_H
