#define DEBUG
//#define NCE

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>

/*
 * Macros
 */

#ifdef DEBUG
#define print(x)    {Serial.print(x);   client.print(x);}
#define println(x)  {Serial.println(x); client.println(x);}
#else
#define print(x)    {client.print(x);}
#define println(x)  {client.println(x);}
#endif

#define HOST_NAME "BCDCLOCK"

/*
 * New types
 */
typedef unsigned long MY_TIME_T;

/*
 * Prototypes
 */
void myDelay(MY_TIME_T waitTime);
void draw(int col, int value);

/*
 * Constants & variables
 */
#define DOWN_UP (0)
#define UP_DOWN (1)
#define SCAN_INTERVAL ((MY_TIME_T)1L)

#ifdef NCE
#define HOST "10.10.15.222"
#define PORT (8752)
#else
#define HOST "10.0.0.101"
#define PORT (8752)
#endif

/*
// WiFi 
#ifdef NCE
const char* ssid = "hsNCE";
const char* password = "";
#else
const char* ssid = MY_SSID;
const char* password = MY_PASSWORD;
#endif
*/

// WiFi console
WiFiClient client;

// Blink
// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin
// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Timestamp
const int timezone = -3;
const int dst = 0;

// Display
//(0,0) is at top-right position of display
// Display pin #      1   7   2   5
const int rows[4] = { 1, 15,  3,  2};
// Display pin #     16  15  11   6  10   4   3
const int cols[7] = {16, 14, 12,  0, 13,  4,  5};

void setup() {
  Serial.begin(115200);
  println("Booting");

  setupWiFiManager();
  setupArduinoOTA();

  println("Ready!");
  print("IP address: ");
  println(WiFi.localIP().toString());

  // WiFi console
  if(!client.connect(HOST, PORT)) {
    print("Could not connect to WiFi console at ");
  } else {
    print("Connected to WiFi console at ");  
  }
  print(HOST);
  print(":");
  println(PORT);
  delay(1000);

  // blink
  println("Waiting for program upload command.");
  pinMode(ledPin, OUTPUT);
  for(int i=0; i<10; i++) {
    ArduinoOTA.handle();
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
    myDelay(1000L);
  }
  println("No program upload command received.");
  myDelay(1000);  // delay to wait all messages been flushed before reuse serial pins
  for(int i=0; i< 4; i++) {
    pinMode(rows[i], OUTPUT);
  }
  for(int i=0; i< 7; i++) {
    pinMode(cols[i], OUTPUT);
  }

  // internal time
  configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");
  println("\nWaiting for time");
  while (!time(nullptr)) {
    print(".");
    delay(1000);
  }
  println("");
}

void loop() {
  ArduinoOTA.handle();
  
  time_t now;
  struct tm *timeinfo;
  time(&now);
  timeinfo = localtime(&now);

#ifdef DEBUG    
  //Print complete date:
  String currentDate;
  currentDate = String("\r");
  currentDate += String(timeinfo->tm_mday) + "/" + String(timeinfo->tm_mon +1) + "/" + String(timeinfo->tm_year + 1900);
  currentDate += " - ";
  currentDate += String(timeinfo->tm_wday +1);
  currentDate += " - ";
  currentDate += String(timeinfo->tm_hour) + ":" + String(timeinfo->tm_min) + ":" + String(timeinfo->tm_sec);
  currentDate += " ";   
  print(currentDate);
#endif
/*
  for(int col=0; col<7; col++) {
    for(int valor=1; valor<10; valor<<=1) {
      draw(col, valor);
    }
  }

  // seconds - cols 0 1
  draw(0,5);
*/

  draw(0, timeinfo->tm_sec % 10);
  draw(1, (timeinfo->tm_sec / 10) % 10);
  // minutes - cols 2 3
  draw(2, timeinfo->tm_min % 10);
  draw(3, (timeinfo->tm_min / 10) % 10);
  // hours - cols 4 5
  draw(4, timeinfo->tm_hour % 10);
  draw(5, (timeinfo->tm_hour / 10) % 10);
  // col 6 not used (yet!)

  // day of week col 7
  draw(6, (timeinfo->tm_wday % 10)+1);

/*
    // year - cols 0 1 2 3
    draw(0, UP_DOWN, (timeinfo->tm_year + 1900) % 10);
    draw(1, UP_DOWN, ((timeinfo->tm_year + 1900) / 10) % 10);
    draw(2, UP_DOWN, ((timeinfo->tm_year + 1900) / 100) % 10);
    draw(3, UP_DOWN, (timeinfo->tm_year + 1900) / 1000);
    // month - cols 4 5
    draw(4, DOWN_UP, timeinfo->tm_mon % 10);
    draw(5, DOWN_UP, timeinfo->tm_mon / 10);
    // month - cols 4 5
    draw(6, DOWN_UP, timeinfo->tm_mday % 10);
    draw(7, DOWN_UP, timeinfo->tm_mday / 10);
*/    
}

/**
 * draw: draws a number into the display
 * This function lights up a LED once a time for SCAN_INTERVAL milliseconds, then turns it off
 * Parameters:
 *  col: column number, from right to left, from 0 to 7
 *  dir: UP_DOWN or DOWN_UP
 *  value: 1 digit unsigned int
 * Returns:
 *  void
  // Display pin #      1   7   2   5
  const int rows[8] = { 0, 16, 15, 12};
  // Display pin #     16  15  11   6   4   3  10
  const int cols[8] = {14, 13,  2,  5,  3,  1,  4};
 */
void draw(int col, int value) {
#ifdef DEBUG  
  int aux = value;
#endif

  for(int row=0; value; row++, value >>= 1) {
    if(value & 1) {
#ifdef DEBUG        
      char s[100];
      sprintf(s, "Value = %d (%d), linha %d, coluna %d", aux, value, 3-row, col);
      println(s);
#endif        
      digitalWrite(rows[3-row], HIGH);
      digitalWrite(cols[col], LOW);
      myDelay(SCAN_INTERVAL);
      digitalWrite(rows[3-row], LOW);
      digitalWrite(cols[col], HIGH);
    }
  }
}

/**
 * myDelay: busy-wait delay compatible with OTA
 * Parameters:
 *  waitTime: unsigned long delay time
 * Returns:
 *  void
*/
void myDelay(MY_TIME_T waitTime) {
  MY_TIME_T previousMillis;
  previousMillis = (MY_TIME_T)millis();
  while((MY_TIME_T)millis() - previousMillis < waitTime) {
    ArduinoOTA.handle();
    delay(1);
  }
}






/**
 * setupWiFiManager
 * Configura o WiFi Manager
 * Utiliza o endereço IP 192.168.0.1
 * SSID OTA-Template
 * Não tem senha 
 */
void setupWiFiManager(void) {
  // WiFiManager, Local initialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  // wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // Apagar as credenciais Wi-Fi armazenadas (descomente a linha abaixo para limpar)
  // wm.resetSettings();
  //set custom ip for portal
  wm.setAPStaticIPConfig(IPAddress(192,168,0,1), IPAddress(192,168,0,1), IPAddress(255,255,255,0));
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  // res = wm.autoConnect("OTA-Template", "password"); // password protected ap
  res = wm.autoConnect(HOST_NAME); // no password protected

  if (!res) {
    println("Failed to connect");
    // ESP.restart();
  }
  else {
    // if you get here you have connected to the WiFi
    println("connected...yeey :)");
  }
}



/**
 * setupArduinoOTA
 * Configura o Arduino OTA para upload de código via Wi-Fi
 * Porta 8266 (default)
 * Host OTA-Template
 * Sem senha
 */
void setupArduinoOTA(void) {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname((HOST_NAME + WiFi.localIP().toString()).c_str());

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    println("Start updating " + type); 
  });

  ArduinoOTA.onEnd([]() { println("\nEnd"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { 
    printf("Progress: %u%%\r", (progress / (total / 100))); 
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      println("End Failed");
    } 
  });

  ArduinoOTA.begin();
  println("Ready");
  print("IP address: ");
  println(WiFi.localIP());
}