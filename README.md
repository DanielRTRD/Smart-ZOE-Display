# SmartED-Display, based on ZOEdisplay from premultiply

An onboard realtime status display for SmartED vehicles

The hardware I used:

* Arduino Uno Rev3
* CAN-BUS Shield https://www.seeedstudio.com/CAN-BUS-Shield-V1-2-p-2256.html
* Arduino LCD KeyPad Shield (SKU: DFR0009) https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)
* Housing (green, modified CAN connector section, USB connector section enlarged): https://www.ebay.de/itm/Großes-Plexiglas-Gehäuse-für-Arduino-Uno-und-Duemilanove-mit-Shield/113382565122?ssPageName=STRK%3AMEBIDX%3AIT&var=413633995787&_trksid=p2057872.m2749.l2649
* any protoshield (to lift the LCD shield)

For operation, it is necessary that the pin 10 of the display in the direction of Can-Shield / Arduino is simply pinched off or bent so that it does not contact down.

The bridge, as it is written on the ZOE display, is not absolutely necessary.

The code is executable, tested and shows everything correctly.

78% of the memory is occupied, whether an OLED graphic LCD still fits needs to be tested.

Activate filters and masks to quickly get the necessary data.

geplanter Displaytyp ( Oled SSD1309 2,42")

Pin-usage:  
A0    Display Shield   Buttons (select, up, right, down and left)  
D3    Display Shield   Backlit Control  
D4 	  Display Shield   DB4  
D5   	Display Shield   DB5  
D6   	Display Shield   DB6  
D7   	Display Shield   DB7  
D8   	Display Shield   RS (Data or Signal Display Selection)  
D9 	  Display Shield   Enable  
D10   CAN-Bus Shield   CS
