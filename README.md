
# Arduino Signal Driver for Scale Trains / Control de semáforos de trenes a escala con Arduino
v 1.0 - Darío Calvo Sánchez, 2021

ENGLISH / _ESPAÑOL_

<br/>

![Set-up scheme](/images/setup.jpg)

<br/>

## DESCRIPTION / _DESCRIPCIÓN_

<br/>

Arduino-based signal control software for LED scale railroad models, using PCA9685 PWM boards, with the following key characteristics:

* Control of up to 12 signals with a maximum of 48 lights. Practically any signal can be controlled:
  * In common-anode configuration, the maximum number of lights per signal is 4 (up to 3 of which can be lit up simultaneously)
  * In common-cathode configuration, the maximum number of lights per signal is 5 (all of which can be lit up simultaneously if required)
* Compatible with both analogue and digital systems
* A mix of common-anode and common-cathode signals can be connected to the system
* A mix of signals with different number of lights can be connected to the same or different PCA9685 boards. Even one signal can have some lights connected to one board and the rest to a different one
* All lights in a common-anode signal will have the same brightness, no matter the number of lights switched on simultaneously on that signal
* Selectable dimming rate, independent of the number of signals or lights connected, allowing realistic representation of any signal type
* Selectable blinking rate
* All commanded lights in a signal will light up or turn off at the same time
* No practical limit in the number of PCA9685 boards that can be connected (up to its maximum of 62)
* Definitions are included for typical Spanish RENFE/ADIF signals, although any signal from any railroad can be defined by the user
* Rocrail SVG files included

<br/>

_Programa de control de señales LED basado en Arduino y placas de señal PWM PCA9685, con las siguientes características:_

* _Control de hasta 12 señales con un máximo de 48 focos. En la práctica puede manejar casi cualquier tipo de señal:_
  * _En señales de ánodo común, el número máximo de focos por señal es de 4, de los cuales 3 podrán estar encendidos simultáneamente._
  * _En señales de cátodo común, el número máximo de focos por señal es de 5, los cuales podrán encenderse todos simultáneamente en caso necesario._
* _Compatible con sistemas analógicos y digitales._
* _Se puede conectar una mezcla de señales de ánodo común y cátodo común sin ningún problema._
* _Se pueden conectar señales con diferente número de focos o configuraciones a la misma o a diferentes placas PCA9685. Incluso una señal puede tener algunos focos conectados a una de las placas y el resto a otra placa._
* _Todos los focos de una señal de ánodo común brillarán con la misma intensidad, no importando cuántos de ellos estén encendidos simultaneámente en la señal._
* _Atenuación/encendido progresivo ajustable e independiente del número de señales o focos conectados, permitiendo una representación realista de cualquier señal luminosa._
* _Velocidad de parpadeo ajustable._
* _Todos los focos o aspectos de una misma señal se encenderán o apagarán simultáneamente._
* _Sin límite práctico en el número de placas PCA9685 que se pueden conectar (hasta su máximo de 62)_
* _Incluye definiciones para las señales típicas de RENFE/ADIF, si bien permite la definición de cualquier sistema de señales de cualquier compañía a voluntad._
* Se incluyen archivos SVG compatibles con Rocrail.

<br/>


## INSTRUCTIONS OF USE / _INSTRUCCIONES DE USO_

<br/>

See instructions and other documentation inside "docs" folder.

<br/>

_Ver documentación e instrucciones en la carpeta "docs"._

_Más información en español en [https://elgajoelegante.com/trenes/](https://elgajoelegante.com/trenes/)_

<br/>

## LICENSE / _LICENCIA_

<br/>

Control de semáforos de trenes a escala con Arduino by Dario Calvo Sanchez is licensed under CC BY-NC-SA 4.0. To view a copy of this license, visit [https://creativecommons.org/licenses/by-nc-sa/4.0](https://creativecommons.org/licenses/by-nc-sa/4.0)