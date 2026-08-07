#include <LCDWIKI_GUI.h> // Core graphics library
#include <LCDWIKI_KBV.h> // Hardware-specific library
#include <TouchScreen.h> // Touchscreen support

// Initialize TFT screen
LCDWIKI_KBV tft(320, 480, A6, A4, A2, A0, A8);//width,height,cs,cd,wr,rd,reset

// Colors
#define BLACK        0x0000
#define BLUE         0x001F
#define RED          0xF800
#define GREEN        0x07E0
#define CYAN         0x07FF
#define MAGENTA      0xF81F
#define YELLOW       0xFFE0
#define WHITE        0xFFFF
#define NAVY         0x000F
#define DARKGREEN    0x03E0
#define DARKCYAN     0x03EF
#define MAROON       0x7800
#define PURPLE       0x780F 
#define OLIVE        0x7BE0 
#define GREY         0xC618 
#define DARKGREY     0x7BEF
#define ORANGE       0xFD20
#define GREENYELLOW  0xAFE5
#define PINK         0xF81F

// Touchscreen pins
#define YP A2
#define XM A3
#define YM 8
#define XP 9
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Calibrated values from touch test
#define TS_LEFT  93
#define TS_RT    926
#define TS_TOP   879
#define TS_BOT   154

// Pin definitions
#define energies_pin 17
#define feed_pin 10
#define hopper_pin 12
#define tray_pin 11
#define drumCont 46

// Serial communication for RPM data
#define rpmSerial Serial1

TSPoint p;
    
//Controls
int feed_nobe = A13;
int hopper_nobe = A11;
int tray_nobe = A9;

int feed_nobe_val;
int hopper_nobe_val;
int tray_nobe_val;

int * feed_val = &feed_nobe_val;
int * hopper_val = &hopper_nobe_val;
int * tray_val = &tray_nobe_val;

//Sensors
int staticVolt_meter = A1;
int feed_meter = A3;
int hopper_meter = A5;
int tray_meter = A7;
int drum_meter;

bool isRunning = false;
unsigned long lastUpdateTime = 0;
unsigned long * lastUpdateTime_P = &lastUpdateTime;
unsigned long TouchInt = 0;
unsigned long * TouchInt_P = &TouchInt;
const unsigned long updateInterval = 2000;
const unsigned long * updateInterval_P = &updateInterval;

unsigned long distime = 0;
unsigned long * dis_t = &distime;

// RPM variables 
int rawDrumSpeed = 0;        // Raw RPM value from Arduino Pro Mini
int drumSpeed = 0;           // Calibrated RPM value for display
unsigned long lastRpmReceived = 0;  // Timestamp of last RPM data received

int * drumSpeed_P = &drumSpeed;
double electrostatic = 0.00;
double * electrostatic_P = &electrostatic;
int feedSpeed = 1;
int * feedSpeed_P = &feedSpeed;
int hopperVib = 1;
int * hopperVib_P = &hopperVib;
int trayVib = 1;
int * trayVib_P = &trayVib;

// Cache previous values to reduce flickering
int lastDrumSpeed;
int * lastDrumSpeed_P = &lastDrumSpeed;
float lastElectrostatic;
float * lastElectrostatic_P = &lastElectrostatic;
float lastFeedSpeed;
float * lastFeedSpeed_P = &lastFeedSpeed;
float lastHopperVib;
float * lastHopperVib_P = &lastHopperVib;
float lastTrayVib;
float * lastTrayVib_P = &lastTrayVib;

// Positions
int yStart = 100;
int * yStart_P = &yStart;
int spacing = 40;
int * spacing_P = &spacing;
int valueX = 200;
int * valueX_P = &valueX;
int unitX = 290;
int * unitX_P = &unitX;

//  RPM Data Reading Function
void readRPMData() {
  // Check for incoming RPM data from Arduino Pro Mini
  if (rpmSerial.available()) {
    String rpmString = rpmSerial.readStringUntil('\n');
    rawDrumSpeed = rpmString.toInt();
    
    // Apply calibration factor only once when receiving new data
    drumSpeed = rawDrumSpeed * 0.3289;  // Your calibration factor
    
    lastRpmReceived = millis();
  }
  
  // Timeout handling - set RPM to 0 if no data received for 1 second
  if (millis() - lastRpmReceived > 1000) {
    drumSpeed = 0;
    rawDrumSpeed = 0;
  }
}

void showIntroScreen() {
  tft.Fill_Screen(BLACK);
  tft.Set_Text_Back_colour(BLACK);
  tft.Set_Text_colour(CYAN);
  tft.Set_Text_Size(6);
  tft.Print_String("BETAIL.NG", CENTER, 130);

  delay(3000); // show for 3 seconds
}

void drawStaticLabels() {
  tft.Set_Text_colour(WHITE);
  tft.Set_Text_Back_colour(BLACK);
  tft.Set_Text_Size(2);

  // Labels
  tft.Print_String("Drum speed:", 10, *yStart_P);
  tft.Print_String("Electrostatic:", 10, *yStart_P + *spacing_P);
  tft.Print_String("Feeding speed:", 10, *yStart_P + 2 * *spacing_P);
  tft.Print_String("Hopper vibrator:", 10, *yStart_P + 3 * *spacing_P);
  tft.Print_String("Tray vibrator:", 10, *yStart_P + 4 * *spacing_P);

  // Units (static)
  tft.Print_String("RPM", *unitX_P, *yStart_P);
  tft.Print_String("kV", *unitX_P, *yStart_P + *spacing_P);
  tft.Print_String("RPM", *unitX_P, *yStart_P + 2 * *spacing_P);
  tft.Print_String("%", *unitX_P, *yStart_P + 3 * *spacing_P);
  tft.Print_String("%", *unitX_P, *yStart_P + 4 * *spacing_P);
}

void updateValues() {
  tft.Set_Text_Mode(1);
  tft.Set_Text_colour(WHITE);
  tft.Set_Text_Back_colour(BLACK);
  tft.Set_Text_Size(2);

  char buf[6];
  int boxWidth = 80; // width of the value display box (pixels)
  int charWidth = 6 * tft.Get_Text_Size(); // width of one char
  int x;

  if (*drumSpeed_P != *lastDrumSpeed_P) {
    sprintf(buf, "%3d", *drumSpeed_P); // Display as integer RPM
    int textWidth = strlen(buf) * charWidth;
    x = valueX + boxWidth - textWidth;
    tft.Fill_Rect(*valueX_P, *yStart_P, boxWidth, *spacing_P, BLACK);
    tft.Print_String(buf, x, *yStart_P);
    *lastDrumSpeed_P = *drumSpeed_P;
  }
  
  if (*electrostatic_P != *lastElectrostatic_P) {
    dtostrf(*electrostatic_P, 5, 2, buf);
    int textWidth = strlen(buf) * charWidth;
    x = valueX + boxWidth - textWidth;
    tft.Set_Text_colour(isRunning ? RED : WHITE);
    tft.Fill_Rect(*valueX_P, *yStart_P + *spacing_P, boxWidth, *spacing_P, BLACK);
    tft.Print_String(buf, x, *yStart_P + *spacing_P);
    *lastElectrostatic_P = *electrostatic_P;
    tft.Set_Text_colour(WHITE);
  }
  
  if (*feedSpeed_P != *lastFeedSpeed_P) {
    dtostrf(*feedSpeed_P, 5, 2, buf);
    int textWidth = strlen(buf) * charWidth;
    x = valueX + boxWidth - textWidth;
    tft.Fill_Rect(*valueX_P, *yStart_P + 2 * *spacing_P, boxWidth, *spacing_P, BLACK);
    tft.Print_String(buf, x, *yStart_P + 2 * *spacing_P);
    *lastFeedSpeed_P = *feedSpeed_P;
  }

  if (*hopperVib_P != *lastHopperVib_P) {
    sprintf(buf, "%3d", *hopperVib_P); // fixed-width 3-digit integer
    int textWidth = strlen(buf) * charWidth;
    x = valueX + boxWidth - textWidth;
    tft.Fill_Rect(*valueX_P, *yStart_P + 3 * *spacing_P, boxWidth, *spacing_P, BLACK);
    tft.Print_String(buf, x, *yStart_P + 3 * *spacing_P);
    *lastHopperVib_P = *hopperVib_P;
  }

  if (*trayVib_P != *lastTrayVib_P) {
    sprintf(buf, "%3d", *trayVib_P);
    int textWidth = strlen(buf) * charWidth;
    x = valueX + boxWidth - textWidth;
    tft.Fill_Rect(*valueX_P, *yStart_P + 4 * *spacing_P, boxWidth, *spacing_P, BLACK);
    tft.Print_String(buf, x, *yStart_P + 4 * *spacing_P);
    *lastTrayVib_P = *trayVib_P;
  }
}

void show_string(uint8_t *str,int16_t x,int16_t y,double csize,uint16_t fc, uint16_t bc,boolean mode)
{
    tft.Set_Text_Mode(mode);
    tft.Set_Text_Size(csize);
    tft.Set_Text_colour(fc);
    tft.Set_Text_Back_colour(bc);
    tft.Print_String(str,x,y);
}

void getVal(double * a_val, int aveg, int log_pin){
  double sum = 0.0;
  for (int i=0; i<aveg; i++){
    sum = sum + analogRead(log_pin);
    delay(5);
  }
  *a_val = sum/aveg;
}

void getValint(int * a_val, int aveg, int log_pin){
  int sum = 0.0;
  for (int i=0; i<aveg; i++){
    sum = sum + analogRead(log_pin);
    delay(5);
  }
  *a_val = sum/aveg;
}

void randomizeValues() {
  getVal(&electrostatic, 15, staticVolt_meter);

  //  Removed the problematic line that was continuously modifying drumSpeed
  // *drumSpeed_P = *drumSpeed_P * 0.3289;  // REMOVED THIS LINE
  
  *electrostatic_P = (*electrostatic_P/1023 * 5.00) * 75.00;

// Hopper Vibration Control
  *hopperVib_P = map(*hopperVib_P, 0, 128, 0, 178);
  *hopper_val = map(*hopper_val, 100, 975, 0, 178);
  
  int pinWriter = *hopperVib_P;
  if (*hopper_val > pinWriter) {
    while (*hopper_val > pinWriter) pinWriter++;
  }
  else if (*hopper_val < pinWriter) {
    while (*hopper_val < pinWriter) pinWriter--;
  }
  pinWriter = constrain(pinWriter,0,178);
  analogWrite(hopper_pin, pinWriter);
  *hopperVib_P = map(pinWriter,0,178,0,100);
  
// Tray Vibration Control
  *trayVib_P = map(*trayVib_P, 0, 128, 0, 178);
  *tray_val = map(*tray_val, 100, 975, 0, 178);
  
  pinWriter = *trayVib_P;
  if (*tray_val > pinWriter) {
    while (*tray_val > pinWriter) pinWriter++;
  }
  else if (*tray_val < pinWriter) {
    while (*tray_val < pinWriter) pinWriter--;
  }
  pinWriter = constrain(pinWriter,0,178);
  analogWrite(tray_pin, pinWriter);
  *trayVib_P = map(pinWriter,0,178,0,100);

// Feed Speed Control
  *feedSpeed_P = map(*feedSpeed_P, 120, 150, 0, 255);
  *feed_val = map(*feed_val, 0, 1000, 0, 255);
  
  pinWriter = *feedSpeed_P;
  if (*feed_val > pinWriter) {
    while (*feed_val > pinWriter) pinWriter++;
  }
  else if (*feed_val < pinWriter) {
    while (*feed_val < pinWriter) pinWriter--;
  }
  pinWriter = constrain(pinWriter,0,255);
  analogWrite(feed_pin, pinWriter);
  *feedSpeed_P = map(pinWriter,0,255,0,18);
}

void metering(){
  ReadWrite(&feedSpeed, feed_meter, &feed_nobe_val, feed_nobe, feed_pin);
  ReadWrite(&hopperVib, hopper_meter, &hopper_nobe_val, hopper_nobe, hopper_pin);
  ReadWrite(&trayVib, tray_meter, &tray_nobe_val, tray_nobe, tray_pin);
}

int ReadWrite(int * meterVal, int meterPin, int * nodeVal, int NobePin, int pin){
  int sum = 0;  // Initialize sum
  
  for (int i=0; i<5; i++){
    sum = sum + analogRead(NobePin);
    delay(20);
  }
  *nodeVal = sum/5;
  
  sum = 0;  // Reset sum
  for (int i=0; i<10; i++){
    sum = sum + analogRead(meterPin);
    delay(20);
  }
  *meterVal = sum/10;
  
  return 0; // Added return value
}

void setup() {
  // Initialize serial communication for RPM data
  rpmSerial.begin(9600);
  
  // Initialize pins
  pinMode(energies_pin, OUTPUT);
  pinMode(feed_pin, OUTPUT);
  pinMode(hopper_pin, OUTPUT);
  pinMode(tray_pin, OUTPUT);
  
  // Initialize display
  tft.Init_LCD();
  tft.Set_Rotation(1);

  showIntroScreen();

  tft.Fill_Screen(BLACK);

  tft.Set_Text_colour(CYAN);
  tft.Set_Text_Back_colour(BLACK);
  tft.Set_Text_Size(3);
  tft.Print_String("Electrostatic", CENTER, 10);
  tft.Print_String("Separator", CENTER, 45);

  tft.Draw_Line(0, 80, 480, 80);
  tft.Draw_Line(335, 80, 335, 320);

  drawStaticLabels();
  updateValues();
  distime = millis();
  
  // Initialize RPM timing
  lastRpmReceived = millis();
}

void loop() {
  //Read RPM data from Arduino Pro Mini
  readRPMData();

  // Read control inputs and update outputs
  metering();
  
  // Drum speed control from potentiometer
  int sum = 0;
  for(int i=0; i<10; i++){
    sum += analogRead(A15);
    delay(10);
  }
  sum /= 10;
  sum = map(sum, 0, 1023, 0, 130);
  analogWrite(drumCont, sum);

  // Update display every 150ms
  if ((millis()-*dis_t) > 150){
      randomizeValues();
      updateValues();
      *dis_t = millis();
      
      // Optional: Debug output to monitor RPM communication
      // Serial.print("Raw RPM: "); Serial.print(rawDrumSpeed);
      // Serial.print(" Calibrated: "); Serial.println(drumSpeed);
  }
}