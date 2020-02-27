# Driver Display
#### Dave Yonkers, 2020

## Dependencies

There are several dependencies for the driver display code. External libraries for the Neopixels and the Adafruit GFX should
be included in the Teensyduino install, but you will need to install the optimized ILI9341 library. Here are the links:
* [ILI9341_t3n](https://github.com/KurtE/ILI9341_t3n)
* [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)
* [Adafruit Neopixel](https://github.com/adafruit/Adafruit_NeoPixel)
* [FlexCAN T4](https://github.com/tonton81/FlexCAN_T4)

You will also need to install pretty much every library in Dave's
[SR-Libraries](https://github.com/msfrt/SR-Libraries) repository.

## InfoScreen Usage

The info screen is the OG driver display screen. Each screen can display four StateSignals. It's pretty freakin simple. The labels are simple `String` objects that you define right in the initialization. They can technically be 8 characters, because that's how many characters can fit on one of these screens, *however*, I'd keep it to 4 or 5 characters __including `:`__. The rest of the space on the line is occupied by the StateSignal itself. You must do the formatting yourself, unfortunately. It's done with cstring formatting and the `sprintf` function. You can read about the parameters [here](http://www.cplusplus.com/reference/cstdio/printf/). Other than that, it should be pretty self-explanatory where you pass in your screen object and StateSignals.

Here's the format of the constructor:

```cpp
InfoScreen my_infoscreen(ILI9341_t3n, StateSignal, StateSignal, StateSignal, StateSignal,
                         /* label */  "LBL1:",     "LBL2:",     "LBL3:",     "LBL4:",
  /* string formatting char arrays*/  char*,       char*,       char*,       char*);
```

And here's a complete example of an `InfoScreen` initialization, assuming that the screen and StateSignal objects have already been properly initialized:
```cpp
char rpm_form[] = "%4.2f";
char oilp_form[] = "%3.1f";
char engt_form[] = "%4.1f";
char battv_form[] = "%4.2f";
InfoScreen engine_screen(display_right, M400_rpm, M400_oilPressure, M400_engineTemp, M400_batteryVoltage,
                           /* label */  "RPM:",   "OILP:",          "ENG:",          "BAT:",
               /* string formatting */  rpm_form, oilp_form ,       engt_form,       battv_form );

```

### Begin an InfoScreen

In order for an info screen to properly display, you must clear the screen and write the formatting details. This is simply done by calling the `begin()` member function. In my code, here, I do this primarily in the `screen_mode_begins` function, which is conveniently located at the bottom of the main file.

### Update an InfoScreen

Just like everything else, updating the variables on the screen is pretty simple. The functionality isn't perfect, but calling the `update()` member function should only write new data to the screen if the number changes. So far, from what I've tested, it works pretty well except when there are some digits of precision that may not be on the screen that get updated. Like, if my character formatting string looks like `"%4.2f"` and `my_var` goes from `10.23` to `10.24`, the screen may still update, even though you've only allotted 4 characters (`10.2`) for the screen. I'm working on this.

### Factoring a variable for an InfoScreen.

What happens when you have a variable, like RPM, that gets into the 10's of thousands? Well, you can factor it! On our driver display, I have RPM factored by 0.001 and I output 2 digits of precision. This allows RPMs like 13758 to be displayed as `13.7` or RPMs like 2567 to be displayed like `2.56`.

To factor a variable, you must set the inverse-factor after the `InfoScreen` initialization and in your `setup()` function. You should probably do it before you call `begin`, too. Here's an example from my engine vitals screen earlier:

```cpp
engine_vitals_right_screen.inv_factor_sig1 = 1000;
```
*note that signals are indexed starting at 1*



## NumberDisplay Usage

Number displays are useful to inform the driver of signals that take an integer value anywhere from **-10 to 10**. You can not use a big number display to show values that have an absolute value greater than 10. If you try to, `N/A` will be printed. A good example of a `NumberDisplay` is a current-gear readout.

Here is how the constructor is set up:
```cpp
NumberDisplay my_big_numbers(ILI9341_t3n, StateSignal, "LABL");
```

And here's an example:
```cpp
NumberDisplay gear_display_left(display_left, M400_gear, "GEAR");
```

You should probably keep the label ≈4 characters. You might get some garbage behavior if it's much longer.

#### Begin a NumberDisplay

Similar to an InfoScreen, you call the `begin()` member function to begin the `NumberScreen`. However, you can pass in a boolean value to the `begin()` function that states whether you should show the startup screen with the big label or not. If you want to just go straight to showing the big number, you can call `begin(false)`. The default is true.

#### Update a NumberDisplay

A `NumberDisplay` is updated in the same fashion as an InfoScreen—with the `update()` member function.


## Lap Time Display Usage

This is **NOT** the display that pops up when a lap trigger is received. To read about how that works, you'll probably just have to read the code. Essentially, that's a function that's continuously ran, but doesn't show anything unless a CAN message is received that sets a delay for the pop-up.

The usage of a lap time display is similar to the other screens, however, you declare global arrays that store the last four laps & lap-times and then pass in a pointer to that array. Then, the screen updates whenever that array changes.

Here is the formatting for the constructor:
```cpp
LapTimeDisplay lapt(ILI9341_t3n, int[], float[], "LABL", bool)
```
The integer array stores the lap number, and the float array stores the time/time difference. Speaking of times/time differences, the boolean value denotes which is in the float array. Technically, the boolean value is assigned to the `colorful` member variable. True essentially means that the float array is of lap differences, and they should be colored. False is just plain ol' lap times.

Here is how I defined the both lap-time display screens:

```cpp
// lap-time array declaration
float prev_lap_times[4];
float prev_lap_times_diff[4];
int prev_lap_numbers[4];
// lap-time display initialization
LapTimeDisplay lap_time_display_left(display_left, prev_lap_numbers, prev_lap_times, "LAP-T", false);
LapTimeDisplay lap_time_display_right(display_right, prev_lap_numbers, prev_lap_times_diff, "LAP-D", true);
```

Every time a lap-trigger comes in over CAN, I update those three arrays.


#### Begin and Update a Lap Time Display

You've heard it enough times now. You begin using `begin()` and update using `update()`. They function in pretty much the same way as the other screen objects.


## Signaling the driver with flashing LEDs

Signaling a driver is quite easy. To do so, all you need to do is set the LED mode to 10, 11, or 12 (or others, if they have been added). You can do this if a particular CAN message is recieved. Say, for example, that OBD determines that the engine may fail soon. If the message that contains that signal is received, all you need to do is set the LED mode to your desired flash.

To turn off the flashing LEDs, you can build in an LED-mode override over CAN, or the driver can simply push the LED-mode change button to acknowledge and mute the warning.

## Sending a message to the driver

The driver display has the ability to read 8-character strings off of CAN. Currently, if the DD recieves message USER_driverSignal (ID 712 on bus 2), it will read each byte of the message into an array as an [ASCII](http://www.asciitable.com) character. Then, similarly to the lap-time popup, it will display the message for a predetermined amount of time (10s, as of me writing this) before it automatically goes away. The driver can acknowledge and remove the message with the push of the page-change button.
