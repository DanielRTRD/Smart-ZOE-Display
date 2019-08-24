# SmartED-Display, based on ZOEdisplay from premultiply

An onboard realtime status display for SmartED vehicles

Die von mir verwendete Hardware:

* Arduino 2009
* CAN-BUS Shield candiy von Watterott http://www.watterott.com/de/Arduino-CANdiy-Shield
* Arduino LCD KeyPad Shield (SKU: DFR0009) https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)

Zum Betrieb ist es notwendig, dass der Pin 10 des Displays richtung Can-Shield/Arduino einfach abgezwickt oder umgebogen wird, so dass er nicht runter kontaktiert.

Die Brücke, wie sie beim ZOEdisplay beschrieben ist, ist nicht zwingend notwendig.

Der Code ist soweit lauffähig, getestet und zeigt alles korrekt an.

Der Code soll noch weiter abgespeckt werden, so dass ein Minimalcode entsteht von dem aus dann weitere Displays und Telemetriemöglichkeiten eingebunden werden.

Aktivieren von Filtern und Masken um schnell die nötigen Daten zuerhalten.

geplante Displaytypen ( Oled SSD1309 2,42", ...)

Pin-Verwendung:  
A0    Display Shield   Buttons (select, up, right, down and left)  
D3    Display Shield   Backlit Control  
D4 	  Display Shield   DB4  
D5   	Display Shield   DB5  
D6   	Display Shield   DB6  
D7   	Display Shield   DB7  
D8   	Display Shield   RS (Data or Signal Display Selection)  
D9 	  Display Shield   Enable  
D10   CAN-Bus Shield   CS
