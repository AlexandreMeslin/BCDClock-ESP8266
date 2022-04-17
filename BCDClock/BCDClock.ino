#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <credentials.h>
#include <NTPClient.h>

/*
 * Prototypes
 */
void draw(int col, int dir, int value);
void print(int s);
void print(char *s);
void print(String s);
void println(int s);
void println(char *s);
void println(String s);

/*
 * Constants & variables
 */
#define DOWN_UP (0)
#define UP_DOWN (1)
#define SCAN_INTERVAL (1)

#define PORT (8752)
#define HOST "10.0.0.101"
 
// WiFi 
const char* ssid = MY_SSID;
const char* password = MY_PASSWORD;

// WiFi console
WiFiClient client;

// Blink
// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin
// Variables will change:
int ledState = LOW;             // ledState used to set the LED
// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

// Timestamp
const int timezone = -3;
const int dst = 0;

// Display
const int rows[8] = {16, 12, 9, 15, 8, 0, 5, 11};
const int cols[8] = {10, 14, 6, 13, 1, 2, 3, 4};

void setup() {
  Serial.begin(115200);
  println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  println("Connected to WiFi");

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("BCDClock");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    printf("Error[%u]: ", error);
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

  // blink
  pinMode(ledPin, OUTPUT);
  for(int i=0; i< 8; i++) {
//    pinMode(rows[i], OUTPUT);
//    pinMode(cols[i], OUTPUT);
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
  unsigned long currentMillis;

  ArduinoOTA.handle();

  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  
    //Print complete date:
    String currentDate;
    
    time_t now;
    struct tm * timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    print("\r");
    currentDate = String(timeinfo->tm_mday) + "/" + String(timeinfo->tm_mon +1) + "/" + String(timeinfo->tm_year + 1900);
    currentDate += " - ";
    currentDate += String(timeinfo->tm_wday +1);
    currentDate += " - ";
    currentDate += String(timeinfo->tm_hour) + ":" + String(timeinfo->tm_min) + ":" + String(timeinfo->tm_sec);
    currentDate += " ";   
    print(currentDate);

    // seconds - cols 0 1
    draw(0, DOWN_UP, timeinfo->tm_sec % 10);
    draw(1, DOWN_UP, timeinfo->tm_sec / 10);
    // minutes - cols 2 3
    draw(2, DOWN_UP, timeinfo->tm_min % 10);
    draw(3, DOWN_UP, timeinfo->tm_min / 10);
    // hours - cols 4 5
    draw(4, DOWN_UP, timeinfo->tm_hour % 10);
    draw(5, DOWN_UP, timeinfo->tm_hour / 10);

    // col 6 not used (yet!)

    // day of week col 7
    draw(7, DOWN_UP, timeinfo->tm_wday);

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
  }
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
 */
void draw(int col, int dir, int value) {
  if(dir == DOWN_UP) {
    for(int row=0; value; row++, value >>= 1) {
      if(value & 1) {
        digitalWrite(rows[row], HIGH);
        digitalWrite(cols[col], LOW);
        myDelay(SCAN_INTERVAL);
        digitalWrite(rows[row], LOW);
        digitalWrite(cols[col], HIGH);
      }
    }
  } else {
    
  }
}

/**
 * myDelay: busy-wait delay compatible with OTA
 * Parameters:
 *  waitTime: unsigned long delay time
 * Returns:
 *  void
 */
void myDelay(unsigned long waitTime) {
  unsigned long previousMillis;
  double foo = 0;

  previousMillis = millis();
  while(millis() - previousMillis < waitTime) {
    ArduinoOTA.handle();
  }
}

void print(String s) {
  Serial.print(s);
  client.print(s);
}

void print(char *s) {
  Serial.print(s);
  client.print(s);
}

void print(int s) {
  Serial.print(s);
  client.print(s);
}

void println(String s) {
  Serial.println(s);
  client.println(s);
}

void println(char *s) {
  Serial.println(s);
  client.println(s);
}

void println(int s) {
  Serial.println(s);
  client.println(s);
}
