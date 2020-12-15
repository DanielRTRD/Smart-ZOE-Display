# Smart ZOE Display
An onboard realtime status display for Renault ZOE

![Preview](https://github.com/DanielRTRD/Smart-ZOE-Display/blob/master/preview.jpg?raw=true)

## Hardware needed
* Arduino Uno Rev3
* CAN-BUS Shield 
    * For example: https://www.seeedstudio.com/CAN-BUS-Shield-V1-2-p-2256.html
* Arduino LCD KeyPad Shield
    * For example: https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)
* DB9 to OBD2 Cable
    * For example: https://www.seeedstudio.com/DB9-to-OBD2-Cable-With-Switch-p-2872.html
* Optional: any prototype shield to lift the LCD shield

## Software requirements
* https://github.com/premultiply/MCP_CAN_lib
* https://github.com/premultiply/LcdBarGraph 
* https://github.com/rlogiacco/AnalogButtons
* https://github.com/Chris--A/PrintEx
* https://github.com/RobTillaart/Arduino/tree/master/libraries/StopWatch
* http://playground.arduino.cc/Code/Timer1

## Information
For operation, it is necessary that the pin 10 of the display in the direction of CAN-Shield (and Arduino) is simply pinched off or bent so that it does not contact down.

The bridge, as it is written on the ZOE display, is not absolutely necessary.

Activate filters and masks to quickly get the necessary data.

## PIN-Usage:
PIN | Shield | Comment
| --- | --- | --- |
| A0 | Display Shield | Buttons (select, up, right, down and left) |
| D3 | Display Shield | Backlit Control |
| D4 | Display Shield | DB4 |
| D5 | Display Shield | DB5 |
| D6 | Display Shield | DB6 |
| D7 | Display Shield | DB7 |
| D8 | Display Shield | RS (Data or Signal Display Selection) |
| D9 | Display Shield | Enable |
| D10 | CAN-Bus Shield | CS |