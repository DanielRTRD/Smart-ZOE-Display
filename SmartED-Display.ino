/*********************************************************************************************************
  Smart owners assistant display
  based on https://github.com/premultiply/ZOEdisplay/
*********************************************************************************************************/

#include <StopWatch.h>
#include <TimerOne.h>
#include <EEPROM.h>
#include <PrintEx.h>
#include <LiquidCrystal.h>
#include <LcdBarGraph.h>
#include <mcp_can.h>
#include <AnalogButtons.h>

//#include <OneWire.h>
//#include <DallasTemperature.h>


#define CAN_INT 2
#define SPI_CS_PIN 10
#define ANALOG_BUTTON_PIN A0
//#define ONE_WIRE_BUS A1


enum screens : byte {
  SCRN_ODO,     // ODO Anzeige
  SCRN_SOC,     // SOC Anzeige
  SCRN_CRG,     // Lade Anzeige
  SCRN_ECO,     // ECO Anzeige
  SCRN_200,
  SCRN_236,
  SCRN_2D5,
  SCRN_318,
  SCRN_3CE,
  SCRN_3D5,
  SCRN_3D7,
  SCRN_3F2,
  SCRN_408,
  SCRN_412,
  SCRN_418,
  SCRN_423,
  SCRN_443,
  SCRN_448,
  SCRN_504,
  SCRN_508,
  SCRN_512,
  SCRN_518,
  SCRN_END      // Ende
};

enum timer_mode : byte {
  TM_CHARGE,    // charging
  TM_MAINS,     // mains on, ready to charge
  TM_PLUGGED,   // plug connected
  TM_DRIVING    // speed > 0
};


union union64 {
  unsigned char uc[8];   // 8 bit (1 byte) 0 bis 255 / 0 bis (2^8)-1)
  byte           b[8];   // 8 bit (1 byte) 0 bis 255 / 0 bis (2^8)-1)
  uint8_t      ui8[8];   // 8 bit (1 byte) 0 bis 255 / 0 bis (2^8)-1)
  uint64_t       ui64;   // 64 bit (4 byte) 0 to 4,294,967,295 / 0 bis (2^64) - 1)
};


const uint64_t PID_INIT_VALUE = 0;
const byte DAY_BRIGHTNESS = UINT8_MAX;
const screens PAGE_LAST = SCRN_END;
const char timerModeChar[] = "CMPD";

//define custom LCD CGRAM char locations
const byte CHR_Power33       = 0x00;
//note: chars 0x01-0x04 are occupied by LcdBarGraph lib
const byte CHR_Power66          = 0x05;
const byte CHR_KW          = 0x06;
const byte CHR_Power99 = 0x07;


StopWatch sw(StopWatch::SECONDS);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
LcdBarGraph lbg(&lcd, 16, 0, 1);
PrintEx lcdEx = lcd;
MCP_CAN CAN(SPI_CS_PIN);

Button btnRIGHT = Button(0, &btnRIGHTClick);
Button btnUP = Button(99, &btnUPClick, &btnUPHold, 500, 125);
Button btnDOWN = Button(255, &btnDOWNClick, &btnDOWNHold, 500, 125);
Button btnLEFT = Button(407, &btnLEFTClick);
Button btnSELECT = Button(637, &btnSELECTClick, &btnSELECTHold, 2000, UINT16_MAX);
AnalogButtons analogButtons = AnalogButtons(ANALOG_BUTTON_PIN, INPUT);
//OneWire oneWire(ONE_WIRE_BUS);
//DallasTemperature sensors(&oneWire);
//DeviceAddress tempDeviceAddress;


//custom LCD CGRAM bitmaps
byte char_tilde[8] = { // ~
  0b00000,
  0b00000,
  0b01000,
  0b10101,
  0b00010,
  0b00000,
  0b00000,
  0b00000
};
byte char_km[8] = { // km
  0b10100,
  0b11000,
  0b10100,
  0b00000,
  0b11111,
  0b10101,
  0b10101,
  0b00000
};
byte char_kW[8] = { // kW
  0b10100,
  0b11000,
  0b10100,
  0b00000,
  0b10101,
  0b10101,
  0b01010,
  0b00000
};
byte char_gradC[8] = { // °C
  0b11000,
  0b11000,
  0b00111,
  0b01000,
  0b01000,
  0b01000,
  0b00111,
  0b00000
};

byte char_Power33[8] = { 
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b10000,
  0b10000,
  0b00000
};
byte char_Power66[8] = { 
  0b00000,
  0b00000,
  0b00000,
  0b00100,
  0b00100,
  0b10100,
  0b10100,
  0b00000
};
byte char_Power99[8] = { 
  0b00000,
  0b00001,
  0b00001,
  0b00101,
  0b00101,
  0b10101,
  0b10101,
  0b00000
};


//internal pid buffers von Smart
uint64_t pid_0x200 = PID_INIT_VALUE;
uint64_t pid_0x236 = PID_INIT_VALUE;
uint64_t pid_0x2D5 = PID_INIT_VALUE;
uint64_t pid_0x318 = PID_INIT_VALUE;
uint64_t pid_0x3CE = PID_INIT_VALUE;
uint64_t pid_0x3D5 = PID_INIT_VALUE;
uint64_t pid_0x3D7 = PID_INIT_VALUE;
uint64_t pid_0x3F2 = PID_INIT_VALUE;
uint64_t pid_0x408 = PID_INIT_VALUE;
uint64_t pid_0x412 = PID_INIT_VALUE;
uint64_t pid_0x418 = PID_INIT_VALUE;
uint64_t pid_0x423 = PID_INIT_VALUE;
uint64_t pid_0x443 = PID_INIT_VALUE;
uint64_t pid_0x448 = PID_INIT_VALUE;
uint64_t pid_0x504 = PID_INIT_VALUE;
uint64_t pid_0x508 = PID_INIT_VALUE;
uint64_t pid_0x512 = PID_INIT_VALUE;
uint64_t pid_0x518 = PID_INIT_VALUE;


//user PID decoder buffer
uint64_t pid_0xPID = PID_INIT_VALUE;

bool timerEdit = false;
bool priceEdit = false;
bool pidnoEdit = false;
bool freezePID = false;
bool singleByteMode = false;
bool screenRefresh = false;

byte pageno = 0;
byte byteno = 0;
byte timerMode = TM_CHARGE;

//isr var
static volatile byte intCount = 0;

unsigned int LocalTime = 0;
unsigned int ChargeRemainingTime = 0;
unsigned int ChargeBeginTime = 0;
unsigned int ChargeEndTime = 0;
unsigned int selectedPID = 0x69F;

unsigned long energy = 0;

float ChargeBeginKwh = 0.0;
float ChargeEndKwh = 0.0;
float priceKwh = 0.0;
float temperature = 0.0;


void setup()
{
  Serial.begin(115200);
  Serial.println("CAN-Display for SMART-ED");
  //Initialize display
  lcd.begin(16, 2);
  lcd.clear();
  lcd.home();

  lcd.setCursor(5, 0); lcd.print(F("SMART"));
  lcd.setCursor(4, 1); lcd.print(F("Display"));
  delay(1000);

  lcd.clear();
  lcd.home();

  //Initialize CAN shield
  pinMode(SPI_CS_PIN, OUTPUT);
  //CAN.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ);
  CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ);
  
  /*
  //Setup CAN PID filters
  //there are 2 mask in mcp2515, you need to set both of them
  //mask0
  CAN.init_Mask(0, 0, 0x0c000000);
  //filter0
  CAN.init_Filt(0, 0, 0x04ff0000); // 0x400 - 0x7ff
  CAN.init_Filt(1, 0, 0x04ff0000);  
  
  //mask1
  CAN.init_Mask(1, 0, 0x07ff0000);
  //filter1
  CAN.init_Filt(2, 0, 0x03910000);
  CAN.init_Filt(3, 0, 0x02120000); // currently not requiered
  CAN.init_Filt(4, 0, 0x01fd0000); // t = 100 ms
  CAN.init_Filt(5, 0, 0x01f60000); // t = 10 ms
  */
  
  CAN.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted
  pinMode(CAN_INT, INPUT);

  //Button assignments
  analogButtons.add(btnRIGHT);
  analogButtons.add(btnUP);
  analogButtons.add(btnDOWN);
  analogButtons.add(btnLEFT);
  analogButtons.add(btnSELECT);

  //adjust LCD Brightness using OC2B PWM (Timer2)
  pinMode(3, OUTPUT);
  analogWrite(3, DAY_BRIGHTNESS);

  //Load custom character bitmaps
  //lcd.createChar(0, char_tilde);
  lcd.createChar(0, char_Power33);
  lcd.createChar(5, char_Power66);
  lcd.createChar(7, char_Power99);
  //lcd.createChar(5, char_km);
  lcd.createChar(6, char_kW);
  //lcd.createChar(7, char_gradC);

  //Read user-stored Page number from EEPROM
  (EEPROM.read(0x00) <= PAGE_LAST) ? pageno = EEPROM.read(0x00) : pageno = 0;
  EEPROM.get(0x10, priceKwh);
  EEPROM.get(0x20, energy);
  EEPROM.get(0x30, ChargeBeginTime);
  EEPROM.get(0x40, ChargeEndTime);
  EEPROM.get(0x50, ChargeBeginKwh);
  EEPROM.get(0x60, ChargeEndKwh);
  EEPROM.get(0x70, selectedPID);
  EEPROM.get(0x80, timerMode);
  
  if (isnan(priceKwh) || isnan(energy) || isnan(ChargeBeginKwh) || isnan(ChargeEndKwh)) {
    priceKwh = 0.0;
    energy = 0;
    ChargeBeginKwh = 0.0;
    ChargeEndKwh = 0.0;
  }

  //Initialize internal temperature sensor
  //sensors.begin();
  //sensors.getAddress(tempDeviceAddress, 0);
  //sensors.setResolution(tempDeviceAddress, 9);
  //sensors.setWaitForConversion(false);
  //sensors.requestTemperatures();
  ////lastTempRequest = millis();

  //Initialize display refresh timer (Timer1)
  Timer1.initialize(250000);
  Timer1.attachInterrupt(LCD_ISR);

  //Initialize stopwatch
  sw.reset();
}


void LCD_ISR()
{
  intCount++;
}


void btnRIGHTClick()
{
  lcd.clear();
  if (pageno < PAGE_LAST) pageno++;
  else pageno = 0;
  screenRefresh = true;
}


void btnUPClick()
{
  screenRefresh = true;
}


void btnUPHold()
{
  screenRefresh = true;
}


void btnDOWNClick()
{
  screenRefresh = true;
}


void btnDOWNHold()
{
  screenRefresh = true;
}


void btnLEFTClick()
{
  lcd.clear();
  if (pageno > 0) pageno--;
  else pageno = PAGE_LAST;
  screenRefresh = true;
}


void btnSELECTClick()
{
  screenRefresh = true;
}


void btnSELECTHold()
{
  saveState();
  EEPROM.update(0x00, pageno);
}


void saveState()
{
  lcd.clear();
  lcd.home();
  lcd.print(F("Saving state..."));
  EEPROM.put(0x10, priceKwh);
  EEPROM.put(0x20, energy);
  EEPROM.put(0x30, ChargeBeginTime);
  EEPROM.put(0x40, ChargeEndTime);
  EEPROM.put(0x50, ChargeBeginKwh);
  EEPROM.put(0x60, ChargeEndKwh);
  EEPROM.put(0x70, selectedPID);
  EEPROM.put(0x80, timerMode);
  //lcd.clear();
}


void loop()
{
  union64 buf;
  
  static bool lastCharging = false;
  static bool lastMains = false;
  static bool lastPlugged = true;
  static bool lastDriving = false;

  static float energymeter = 0.0;

  //perf counter vars
  static unsigned long startCycle = 0;
  static unsigned long lastCycle = 0;
  static unsigned long minCycle = 0;
  static unsigned long maxCycle = 0;
  static unsigned long countCycle = 0;

  //car status
  static bool isPlugged = false;
  static bool isMains = false;
  static bool isCharging = false;
  static bool isDriving = false;

  //pid decoder timing vars
  static unsigned long lastPidSeen = 0;
  static unsigned long lastPidCycleDuration = 0;

  //perf counter
  countCycle++;
  minCycle = min(minCycle, lastCycle);
  maxCycle = max(maxCycle, lastCycle);
  startCycle = millis();

  //CAN receiver
  if(!digitalRead(CAN_INT)) { //while (CAN_MSGAVAIL == CAN.checkReceive())
    long unsigned int rxId;
    byte len = 0; 

    buf.ui64 = PID_INIT_VALUE;
    CAN.readMsgBuf(&rxId, &len, buf.b);
    //user pid decoder
    if (rxId == selectedPID) {
      lastPidCycleDuration = millis() - lastPidSeen;
      lastPidSeen = millis();
      if (!freezePID) pid_0xPID = swap_uint64(buf.ui64);
    }
    switch (rxId) {
    case 0x200: pid_0x200 = swap_uint64(buf.ui64); break;
    case 0x236: pid_0x236 = swap_uint64(buf.ui64); break;
    case 0x2D5: pid_0x2D5 = swap_uint64(buf.ui64); break;
    case 0x318: pid_0x318 = swap_uint64(buf.ui64); break;
    case 0x3CE: pid_0x3CE = swap_uint64(buf.ui64); break;
    case 0x3D5: pid_0x3D5 = swap_uint64(buf.ui64); break;
    case 0x3D7: pid_0x3D7 = swap_uint64(buf.ui64); break;
    case 0x3F2: pid_0x3F2 = swap_uint64(buf.ui64); break;
    case 0x408: pid_0x408 = swap_uint64(buf.ui64); break;
    case 0x412: pid_0x412 = swap_uint64(buf.ui64); break;
    case 0x418: pid_0x418 = swap_uint64(buf.ui64); break;
    case 0x423: pid_0x423 = swap_uint64(buf.ui64); break;
    case 0x443: pid_0x443 = swap_uint64(buf.ui64); break;
    case 0x448: pid_0x448 = swap_uint64(buf.ui64); break;
    case 0x504: pid_0x504 = swap_uint64(buf.ui64); break;
    case 0x508: pid_0x508 = swap_uint64(buf.ui64); break;
    case 0x512: pid_0x512 = swap_uint64(buf.ui64); break;
    case 0x518: pid_0x518 = swap_uint64(buf.ui64); break;
    }
  }

  //read buttons
  analogButtons.check();

    if (intCount | screenRefresh) {
  //display screens
    screenRefresh = false;
    lcd.home();

    switch (pageno) {

      case SCRN_ODO: // ODO Aneige Reichweite
        lcd.setCursor(2, 0); lcd.print(F("Km:")); lcdEx.printf("%7ukm", (pid_0x412 >> 24 ) & 0xFFFFFFu ); // Kmstand
        lcd.setCursor(0, 1); lcd.print(F("Rw:")); lcdEx.printf("%3ikm", (pid_0x318 >> 0 ) & 0xFFu ); // Reichweite
        //lcd.setCursor(9, 1); lcd.print(F("Pwr")); lcdEx.printf("%3i%%", (pid_0x318 >> 16 ) & 0xFFu ); // Power
        lcd.setCursor(10, 1); lcd.print(F("Pwr: "));
        if ( ((pid_0x318 >> 16 ) & 0xFFu ) == 33 ) {lcd.write(CHR_Power33);} // Power
        if ( ((pid_0x318 >> 16 ) & 0xFFu ) == 66 ) {lcd.write(CHR_Power66);} // Power
        if ( ((pid_0x318 >> 16 ) & 0xFFu ) == 99 ) {lcd.write(CHR_Power99);} // Power
      break;

      case SCRN_CRG: // Lade Anzeige
        if ( ((pid_0x448 >> 56) & 0xFFu) == 0x0F )
        { 
        lcd.setCursor(0, 0); lcd.print(F("Power   ")); lcdEx.printf("%7.1f", ( (((pid_0x448 >> 0) & 0xFFFFu) / 10.0 ) * ((((pid_0x508 >> 32) & 0x3FFFu) / 10 ) - 819.2 ) ) * 0.001 ); lcd.write(CHR_KW); // Leistung
        lcd.setCursor(1, 1); lcdEx.printf("%5.1fV", ((pid_0x448 >> 0) & 0xFFFFu) / 10.0 ); // hvV
        lcd.setCursor(8, 1); lcdEx.printf("%6.1fA", (((pid_0x508 >> 32) & 0x3FFFu) / 10 ) - 819.2 ); // hvA
        }
        else
        {
        lcd.setCursor(0, 0); lcd.print(F("Power  ")); lcdEx.printf("    __._"); lcd.write(CHR_KW); // Leistung
        lcd.setCursor(1, 1); lcdEx.printf("___._V"); // hvV
        lcd.setCursor(8, 1); lcdEx.printf("%6.1fA", (((pid_0x508 >> 32) & 0x3FFFu) / 10 ) - 819.2 ); // hvA
        }
      break;

      case SCRN_SOC: // SOC Anzeige
        lcd.setCursor(2, 0); lcd.print(F(" SOC ")); lcdEx.printf("%5.1f%%", ((pid_0x518 ) & 0xFFu)  /  2.0 ); // SOC
        lcd.setCursor(2, 1); lcd.print(F("rSOC ")); lcdEx.printf("%5.1f%%", ((pid_0x2D5 >> 16) & 0xFFFu) / 10.0 ); // rSOC
      break;

      case SCRN_ECO: // ECO Aneige
        lcd.setCursor(0, 0); lcd.print(F("ECO")); lcdEx.printf("%3i%%", ((pid_0x3F2 >> 32 ) & 0xFFu ) / 2); // ECO
        lcd.setCursor(9, 0); lcd.print(F("bre")); lcdEx.printf("%3i%%", ((pid_0x3F2 >> 40 ) & 0xFFu ) / 2); // ECO
        lcd.setCursor(0, 1); lcd.print(F("drv")); lcdEx.printf("%3i%%", ((pid_0x3F2 >> 48 ) & 0xFFu ) / 2); // ECO
        lcd.setCursor(9, 1); lcd.print(F("acc")); lcdEx.printf("%3i%%", ((pid_0x3F2 >> 56 ) & 0xFFu ) / 2); // ECO
      break;

      case SCRN_200: // PID 0x200
        lcd.print(F("PID 0x200"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x200 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x200 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x200 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x200 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x200 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x200 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x200 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x200 >>  0*8) & 0xFFu);
        break;

      case SCRN_236: // PID 0x236
        lcd.print(F("PID 0x236"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x236 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x236 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x236 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x236 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x236 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x236 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x236 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x236 >>  0*8) & 0xFFu);
        break;

      case SCRN_2D5: // PID 0x2D5
        lcd.print(F("PID 0x2D5"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x2D5 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x2D5 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x2D5 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x2D5 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x2D5 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x2D5 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x2D5 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x2D5 >>  0*8) & 0xFFu);
        break;

      case SCRN_318: // PID 0x318
        lcd.print(F("PID 0x318"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x318 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x318 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x318 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x318 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x318 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x318 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x318 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x318 >>  0*8) & 0xFFu);
        break;

      case SCRN_3CE: // PID 0x3CE
        lcd.print(F("PID 0x3CE"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x3CE >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3CE >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3CE >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3CE >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3CE >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3CE >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3CE >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3CE >>  0*8) & 0xFFu);
        break;

      case SCRN_3D5: // PID 0x3D5
        lcd.print(F("PID 0x3D5"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x3D5 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D5 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D5 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D5 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D5 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D5 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D5 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D5 >>  0*8) & 0xFFu);
        break;

      case SCRN_3D7: // PID 0x3D7
        lcd.print(F("PID 0x3D7"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x3D7 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D7 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D7 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D7 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D7 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D7 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D7 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3D7 >>  0*8) & 0xFFu);
        break;

      case SCRN_3F2: // PID 0x3F2
        lcd.print(F("PID 0x3F2"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x3F2 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3F2 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3F2 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3F2 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3F2 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3F2 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3F2 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x3F2 >>  0*8) & 0xFFu);
        break;

      case SCRN_408: // PID 0x408
        lcd.print(F("PID 0x408"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x408 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x408 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x408 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x408 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x408 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x408 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x408 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x408 >>  0*8) & 0xFFu);
        break;

      case SCRN_412: // PID 0x412
        lcd.print(F("PID 0x412"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x412 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x412 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x412 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x412 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x412 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x412 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x412 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x412 >>  0*8) & 0xFFu);
        break;

      case SCRN_418: // PID 0x418
        lcd.print(F("PID 0x418"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x418 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x418 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x418 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x418 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x418 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x418 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x418 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x418 >>  0*8) & 0xFFu);
        break;

      case SCRN_423: // PID 0x423
        lcd.print(F("PID 0x423"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x423 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x423 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x423 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x423 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x423 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x423 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x423 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x423 >>  0*8) & 0xFFu);
        break;

      case SCRN_443: // PID 0x443
        lcd.print(F("PID 0x443"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x443 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x443 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x443 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x443 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x443 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x443 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x443 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x443 >>  0*8) & 0xFFu);
        break;

      case SCRN_448: // PID 0x448
        lcd.print(F("PID 0x448"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x448 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x448 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x448 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x448 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x448 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x448 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x448 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x448 >>  0*8) & 0xFFu);
        break;

      case SCRN_504: // PID 0x504
        lcd.print(F("PID 0x504"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x504 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x504 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x504 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x504 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x504 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x504 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x504 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x504 >>  0*8) & 0xFFu);
        break;

      case SCRN_508: // PID 0x508
        lcd.print(F("PID 0x508"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x508 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x508 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x508 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x508 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x508 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x508 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x508 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x508 >>  0*8) & 0xFFu);
        break;

      case SCRN_512: // PID 0x512
        lcd.print(F("PID 0x512"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x512 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x512 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x512 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x512 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x512 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x512 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x512 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x512 >>  0*8) & 0xFFu);
        break;

      case SCRN_518: // PID 0x518
        lcd.print(F("PID 0x518"));
        lcd.setCursor(0, 1);
        lcdEx.printf("%02x", (pid_0x518 >>  7*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x518 >>  6*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x518 >>  5*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x518 >>  4*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x518 >>  3*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x518 >>  2*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x518 >>  1*8) & 0xFFu);
        lcdEx.printf("%02x", (pid_0x518 >>  0*8) & 0xFFu);
        break;


      case SCRN_END: // ENDE
        lcd.setCursor(3, 0);
        lcd.print(F("eok gnah's"));
        lcd.setCursor(0, 1);
        lcd.print(F("Smart-ED Display"));
      break;
              
    }
    //perfmon cycle reset
    minCycle = UINT32_MAX;
    maxCycle = 0;
    countCycle = 0;
  }
  lastCycle = millis() - startCycle;
}


uint64_t swap_uint64(uint64_t val)
{
  val = ((val << 8)  & 0xFF00FF00FF00FF00ULL ) | ((val >> 8)  & 0x00FF00FF00FF00FFULL );
  val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
  return (val << 32) | (val >> 32);
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

