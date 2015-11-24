/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "microbit/modmicrobit.h"

STATIC const char *help_text =
"Welcome to MicroPython on the BBC micro:bit!\n";

STATIC const char *help_sayhello =
"Type 'import microbit', press return.\n"
"Next, try this command:"
"  microbit.display.scroll('Hello')\n"
;

STATIC const char *help_tryme =
"  microbit.running_time()\n"
"  microbit.sleep(1000)\n"
"  microbit.button_a.is_pressed()\n"
"What do these commands do? Can you improve them? HINT: use the up and down\n"
"arrow keys to get your command history. Press the TAB key to auto-complete\n"
"unfinished words (so 'mi' becomes 'microbit' after you press TAB). These\n"
"tricks save a lot of typing and look cool!\n"
"\n"
;

STATIC const char *help_explore =
"Explore:\n"
"Type 'help(something)' to find out about it. Type 'dir(something)' to see what\n"
"it can do. For goodness sake, don't type 'import this'.\n"
"\n"
;

STATIC const char *help_stuff =
"Stuff to explore:\n"
"  microbit.accelerometer         -- detect the device's position (orientation)\n"
"  microbit.button_a.is_pressed() -- is button A pressed? (True or False)\n"
"  microbit.button_b.is_pressed() -- is button B pressed? (True or False)\n"
"  microbit.compass               -- detect the device's heading\n"
"  microbit.display               -- display things (pixels, characters, words)\n"
"  microbit.Image                 -- make pictures for the display\n"
"  microbit.pin0                  -- control the gold input/output (IO) pin0\n"
"  microbit.panic()               -- enter panic mode (requires a restart)\n"
"  microbit.random(n)             -- get a random number between 0 and n-1\n"
"  microbit.reset()               -- reset the device\n"
"  microbit.sleep(n)              -- wait for n milliseconds (1 second = 1000)\n"
"  microbit.running_time()        -- get the number of milliseconds since reset\n"
"\n"
;

STATIC const char *help_control =
"Control commands:\n"
"  CTRL-C        -- stop a running program\n"
"  CTRL-D        -- on a blank line, do a soft reset of the micro:bit\n"
"\n"
;

STATIC const char *help_modules =
"Available modules: array, collections, microbit, micropython, gc, struct, sys,\n"
"this\n"
;

STATIC const char *help_web =
"For more information about micro:bit, visit: http://www.microbit.co.uk\n"
"For more information about Python, visit: http://python.org/\n"
"To find out about MicroPython, visit: http://micropython.org/\n"
;

typedef struct _mp_doc_t {
    mp_const_obj_t obj;
    const char *doc;
} mp_doc_t;

STATIC const mp_doc_t help_table_types[] = {
    {&microbit_accelerometer_type, "MicroBitAccelerometer type\n"},
};

// Consistency between messages and minimal jargon improves help text.
STATIC const mp_doc_t help_table_instances[] = {
    {&microbit_module, "Useful stuff to control the micro:bit hardware.\n"},
    // System state objects
    {&microbit_panic_obj, "Put micro:bit in panic() mode and display an unhappy face.\nPress reset button to exit panic() mode.\n"},
    {&microbit_random_obj, "Return a random(number) between 0 and 'number - 1'.\nrandom(10) returns a number between 0 and 9.\n"},
    {&microbit_sleep_obj, "Put micro:bit to sleep(time) for some milliseconds (1 second = 1000 ms) of time.\nsleep(2000) gives micro:bit a 2 second nap.\n"},
    {&microbit_running_time_obj, "Return running_time() in milliseconds since micro:bit's last reset.\n"},
    // Accelerometer 3D orientation
    {&microbit_accelerometer_obj, "Detect micro:bit's movement in 3D.\nIt measures tilt (X and Y) and up-down (Z) motion.\n"},
    {&microbit_accelerometer_get_x_obj, "Return micro:bit's tilt (X acceleration) in milli-g's.\n"},
    {&microbit_accelerometer_get_y_obj, "Return micro:bit's tilt (Y acceleration) in milli-g's.\n"},
    {&microbit_accelerometer_get_z_obj, "Return micro:bit's up-down motion (Z acceleration) in milli-g's.\nZ is a positive number when moving up. Moving down, Z is a negative number.\n"},
    // Pushbutton
    {&microbit_button_a_obj, "micro:bit's 'A' button. When button is pressed down, is_pressed() is True.\n"},
    {&microbit_button_b_obj, "micro:bit's 'B' button. When button is pressed down, is_pressed() is True.\n"},
    {&microbit_button_is_pressed_obj, "If the button is pressed down, is_pressed() is True, else False.\n"},
    // TODO: Add was_pressed, get_presses, reset_presses (FUTURE)
    // Compass 3D direction heading
    {&microbit_compass_obj, "Use micro:bit's compass to detect the direction it is heading in.\nThe compass can detect magnetic fields.\nIt uses the Earth's magnetic field to detect direction.\n"},
    {&microbit_compass_is_calibrated_obj, "If micro:bit's compass is_calibrated() and adjusted for accuracy, return True.\nIf compass hasn't been adjusted for accuracy, return False.\n"},
    {&microbit_compass_calibrate_obj, "If micro:bit is confused, calibrate() the compass to adjust the its accuracy.\n"},
    {&microbit_compass_is_calibrating_obj, "If micro:bit's compass is_calibrating() its accuracy, it sends True.\nIf it's not busy calibrating things, it sends False.\n"},
    {&microbit_compass_clear_calibration_obj, "Reset micro:bit's compass using clear_calibration() command. Run calibrate() to improve accuracy.\n"},
    {&microbit_compass_get_x_obj, "Return magnetic field detected along micro:bit's X axis.\nUsually, the compass returns the earth's magnetic field in micro-Tesla units.\nUnless...a strong magnet is nearby!\n"},
    {&microbit_compass_get_y_obj, "Return magnetic field detected along micro:bit's Y axis.\nUsually, the compass returns the earth's magnetic field in micro-Tesla units.\nUnless...a strong magnet is nearby!\n"},
    {&microbit_compass_get_z_obj, "Return magnetic field detected along micro:bit's Z axis.\nUsually, the compass returns the earth's magnetic field in micro-Tesla units.\nUnless...a strong magnet is nearby!\n"},
    // Display 5x5 LED grid
    {&microbit_display_obj, "micro:bit's 5x5 LED display.\n"},
    {&microbit_display_show_obj, "Use show(s) to print the string 's' to the display. Try show('Hello!').\nUse show(s, i) to show string 's', one character at a time with a delay of 'i' milliseconds.\n"},
    {&microbit_display_scroll_obj, "Use scroll(s) to scroll the string 's' across the display.\nUse scroll(s, i) to scroll string 's' with a delay of 'i' milliseconds after each character.\n"},
    {&microbit_display_clear_obj, "Use clear() to clear micro:bit's display.\n"},
    {&microbit_display_animate_obj, "Use animate(img, delay, stride, start=0, async=False, repeat=False) to animate\n    image 'img' with 'delay' milliseconds and 'stride' pixels offset between\n    frames. Optional: 'start' offset from left hand side, 'async' to run in the\n    background, 'repeat' to loop the animation.\n"},
    {&microbit_display_get_pixel_obj, "Use get_brightness(x, y) to return the display's brightness at LED pixel (x,y).\nBrightness can be from 0 (LED is off) to 9 (maximum LED brightness).\n"},
    {&microbit_display_set_pixel_obj, "Use set_brightness(x, y, b) to set the display at LED pixel (x,y) to brightness 'b'.\nbrightness 'b', which can be set between 0 (off) to 9 (full brightness).\n"},
    // Pins
    {&microbit_p0_obj, "micro:bit's pin 0 on the gold edge connector.\n"},
    {&microbit_p1_obj, "micro:bit's pin 1 on the gold edge connector.\n"},
    {&microbit_p2_obj, "micro:bit's pin 2 on the gold edge connector.\n"},
    {&microbit_p3_obj, "micro:bit's pin 3 on the gold edge connector.\n"},
    {&microbit_p4_obj, "micro:bit's pin 4 on the gold edge connector.\n"},
    {&microbit_p5_obj, "micro:bit's pin 5 on the gold edge connector.\n"},
    {&microbit_p6_obj, "micro:bit's pin 6 on the gold edge connector.\n"},
    {&microbit_p7_obj, "micro:bit's pin 7 on the gold edge connector.\n"},
    {&microbit_p8_obj, "micro:bit's pin 8 on the gold edge connector.\n"},
    {&microbit_p9_obj, "micro:bit's pin 9 on the gold edge connector.\n"},
    {&microbit_p10_obj, "micro:bit's pin 10 on the gold edge connector.\n"},
    {&microbit_p11_obj, "micro:bit's pin 11 on the gold edge connector.\n"},
    {&microbit_p12_obj, "micro:bit's pin 12 on the gold edge connector.\n"},
    {&microbit_p13_obj, "micro:bit's pin 13 on the gold edge connector.\n"},
    {&microbit_p14_obj, "micro:bit's pin 14 on the gold edge connector.\n"},
    {&microbit_p15_obj, "micro:bit's pin 15 on the gold edge connector.\n"},
    {&microbit_p16_obj, "micro:bit's pin 16 on the gold edge connector.\n"},
    {&microbit_p19_obj, "micro:bit's pin 19 on the gold edge connector.\n"},
    {&microbit_p20_obj, "micro:bit's pin 20 on the gold edge connector.\n"},
    {&microbit_pin_write_digital_obj, "micro:bit, write_digital(choice) to the pin. You have two 'choice' values, 0 (lo) or 1 (hi).\n"},
    {&microbit_pin_read_digital_obj, "micro:bit, read_digital() value from the pin as either 0 (lo) or 1 (hi).\n"},
    {&microbit_pin_write_analog_obj, "micro:bit, write_analog(value) to the pin. You can use a whole number between 0 and 255 as the value.\n"},
    {&microbit_pin_read_analog_obj, "micro:bit, read_analog() value from the pin. Wow, analog has lots of values (0 - 65535). Digital has only 0 and 1.\n"},
    {&microbit_pin_is_touched_obj, "If pin is_touched() on micro:bit, return True. If nothing is touching the pin, return False.\n"},
};

STATIC void pyb_help_print_info_about_object(mp_obj_t name_o, mp_obj_t value) {
    mp_printf(&mp_plat_print, "  ");
    mp_obj_print(name_o, PRINT_STR);
    mp_printf(&mp_plat_print, " -- ");
    mp_obj_print(value, PRINT_STR);
    mp_printf(&mp_plat_print, "\n");
}

STATIC mp_obj_t pyb_help(uint n_args, const mp_obj_t *args) {
    if (n_args == 0) {
        // print a general help message
        mp_printf(&mp_plat_print, "%s", help_text);

    } else {
        mp_obj_t args0 = args[0];
        mp_obj_type_t *args0_type = mp_obj_get_type(args0);
        if (args0_type->name == MP_QSTR_bound_method) {
            args0 = ((mp_obj_t*)args0)[1]; // extract method
            args0_type = mp_obj_get_type(args0);
        }

        // see if we have specific help info for this instance
        for (size_t i = 0; i < MP_ARRAY_SIZE(help_table_instances); i++) {
            if (args0 == help_table_instances[i].obj) {
                mp_print_str(&mp_plat_print, help_table_instances[i].doc);
                //if (args0_type == &mp_type_module) {
                //TODO here we can list the things inside the module
                //}
                return mp_const_none;
            }
        }

        // see if we have specific help info for this type
        for (size_t i = 0; i < MP_ARRAY_SIZE(help_table_types); i++) {
            if (args0 == help_table_types[i].obj || args0_type == help_table_types[i].obj) {
                mp_print_str(&mp_plat_print, help_table_types[i].doc);
                return mp_const_none;
            }
        }

        // don't have specific help info, try instead to print something sensible

        mp_printf(&mp_plat_print, "object ");
        mp_obj_print(args0, PRINT_STR);
        mp_printf(&mp_plat_print, " is of type %q\n", args0_type->name);

        mp_map_t *map = NULL;
        if (args0_type == &mp_type_module) {
            map = mp_obj_dict_get_map(mp_obj_module_get_globals(args0));
        } else {
            mp_obj_type_t *type;
            if (args0_type == &mp_type_type) {
                type = args0;
            } else {
                type = args0_type;
            }
            if (type->locals_dict != MP_OBJ_NULL && MP_OBJ_IS_TYPE(type->locals_dict, &mp_type_dict)) {
                map = mp_obj_dict_get_map(type->locals_dict);
            }
        }
        if (map != NULL) {
            for (uint i = 0; i < map->alloc; i++) {
                if (map->table[i].key != MP_OBJ_NULL) {
                    pyb_help_print_info_about_object(map->table[i].key, map->table[i].value);
                }
            }
        }
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_builtin_help_obj, 0, 1, pyb_help);
