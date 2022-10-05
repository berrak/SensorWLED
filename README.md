<!---
![Display](./images/SensorWLED.gif)
-->

[![GitHub license](https://img.shields.io/github/license/berrak/SensorWLED.svg?logo=gnu&logoColor=ffffff)](https://github.com/berrak/SensorWLED/blob/master/LICENSE)
[![Installation instructions](https://www.ardu-badge.com/badge/SensorWLED.svg?)](https://www.ardu-badge.com/SensorWLED)
[![GitHub version](https://img.shields.io/github/release/berrak/SensorWLED.svg?logo=github&logoColor=ffffff)](https://github.com/berrak/SensorWLED/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/berrak/SensorWLED.svg?logo=github&logoColor=ffffff)](https://github.com/berrak/SensorWLED/releases/latest)
[![GitHub stars](https://img.shields.io/github/stars/berrak/SensorWLED.svg?logo=github&logoColor=ffffff)](https://github.com/berrak/SensorWLED/stargazers)
[![GitHub issues](https://img.shields.io/github/issues/berrak/SensorWLED.svg?logo=github&logoColor=ffffff)](https://github.com/berrak/SensorWLED/issues)
[![Documentation](https://img.shields.io/badge/documentation-doxygen-green.svg)](http://berrak.github.io/SensorWLED/)

# Arduino library SensorWLED

The Arduino library `SensorWLED` splits the input `DC varying analog signal` into components. The microcontroller reading from analog input is
divided into library methods which return these as read, while other methods hold the peak value while decaying with user-set decay parameters.

## Why try this library?

- Keep peak value readings on display for humans to record while at the same time following real-time updates in parallel without loss of information.
- The library support exponential- and linear decay rate for slow and fast superimposed DC signals, respectively.
- Create fast-changing VU-meters or equally track DC disturbances occurring widely apart in time.

Use the library without more than regular analog inputs with any suitable display for readings. For the specific purpose of measuring fast-changing Neopixel pixel currents, use the [SensorWLED board](https://github.com/berrak/WLED-DC-Sensor-Board). The varying current level is often only roughly estimated in such WLED projects.

## Set up an ESP8266, ESP32 or other compatible microcontrollers

TBD.

## Run the example

In the Arduino IDE, scroll down the long list below `File->Examples` and find `SensorWLED`.

Firstly, you must include the library in your sketch, instantiate the SensorWLED object and declare two variables to hold peak- and real-time values:
```cpp
#include <SensorWLED.h>
#include <EEPROM.h>

// Instantiate a SensorWLED object
SensorWLED WledOne;

double mv_value ;		// Analog momentanous value
double mv_pk_value ;	// Peak analog value
```
Define in the setup() section required library parameters and call the begin-method, for example:

```cpp
// ESP8266
DataEEPROMType_t ParamsOne = {
	.analog_pin = 0,
	.bits_resolution_adc = bits10,      // For ESP32 use 'bits12'
	.mv_maxvoltage_adc = mv_vcc_3v3,
	.ms_poll_time = 500,
	.ms_hold_time = 250,
	.decay_model = exponential_decay,
	.decay_rate = 0.75,
};

WledOne.begin(ParamsOne);
```

In the loop() function, use the two primary methods to retrieve the display's peak and real-time values.

```cpp
// Current sensor updates every 500 ms (ms_poll_time).
if (WledOne.updateAnalogRead() == true) {
	mv_value = (double) WledOne.getMappedValue();
	mv_pk_value = (double) WledOne.getMappedPeakValue();
	updateSmallDisplay();
}
```

The function `updateSmallDisplay()` is a generic function to display the retrieved values, depending on the display type attached to the microcontroller.

## Documentation (GitHub Pages - Doxygen)

[Library documentation](https://berrak.github.io/SensorWLED/classSensorWLED.html).

## How to Install

Click on the green `Library Manager` badge above for instructions,
or use the alternative manual installation procedure.

1. Navigate to the [Releases page](https://github.com/berrak/SensorWLED/releases).
1. Download the latest released ZIP-archive in `~/Arduino/libraries`.
1. Unzip the archive.
1. Rename the new directory. Remove *version-code*, or *master* in the name like this for `SensorWLED`.
1. Restart Arduino IDE.
1. In Arduino IDE scroll down the long list below `Sketch->Include Library` and find `SensorWLED`.


## Do you like the tiny 'Sensor WLED board'?

You can purchase all the latest designed boards on `Tindie`.

[![Tindie](./images/tindie-small.png)](https://www.tindie.com/stores/debinix/)

We appreciate your support.
