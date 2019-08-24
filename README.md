# SmartED-Display, based on ZOEdisplay from premultiply

An onboard realtime status display for SmartED vehicles

Die von mir verwendete Hardware:

* Arduino R3 mit MircoUSB
* CAN-BUS Shield https://www.seeedstudio.com/CAN-BUS-Shield-V1-2-p-2256.html
* Arduino LCD KeyPad Shield (SKU: DFR0009) https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)
* Gehäuse (grün, modifiziert CAN Steckerausschnitt, USB Steckerausschnitt vergößert): https://www.ebay.de/itm/Großes-Plexiglas-Gehäuse-für-Arduino-Uno-und-Duemilanove-mit-Shield/113382565122?ssPageName=STRK%3AMEBIDX%3AIT&var=413633995787&_trksid=p2057872.m2749.l2649
* irgendwein Protoshield (um das LCD shield anzuheben)

Zum Betrieb ist es notwendig, dass der Pin 10 des Displays richtung Can-Shield/Arduino einfach abgezwickt oder umgebogen wird, so dass er nicht runter kontaktiert.

Die Brücke, wie sie beim ZOEdisplay beschrieben ist, ist nicht zwingend notwendig.

Der Code ist soweit lauffähig, getestet und zeigt alles korrekt an.

Der Speicher ist zu 78% belegt, ob ein OLED Grafik-LCD noch passt muss getestet werden.

Aktivieren von Filtern und Masken um schnell die nötigen Daten zuerhalten.

geplanter Displaytyp ( Oled SSD1309 2,42")

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
