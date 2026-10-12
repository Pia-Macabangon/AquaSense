#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// OLED SETTINGS
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DS18B20
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// SENSOR PINS
const int phPin = A0;
const int turbidityPin = A1;
const int tdsPin = A2;

// OUTPUT PINS
const int greenLED = 8;
const int yellowLED = 9;
const int redLED = 10;
const int buzzer = 11;
const int relayPin = 7;

// VARIABLES
float voltagePH;
float pHValue;

int turbidityValue;
float turbidityNTU;

int tdsValue;
float tdsPPM;

float temperatureC;

void setup() {

  Serial.begin(9600);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  pinMode(buzzer, OUTPUT);

  pinMode(relayPin, OUTPUT);

  // RELAY OFF at startup
  digitalWrite(relayPin, HIGH);

  sensors.begin();

  // OLED START
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAILED");
    while (1);
  }

  display.clearDisplay();

  // BIGGER AQUASENSE
  display.setTextSize(2);
  display.setTextColor(WHITE);

  display.setCursor(10, 25);
  display.println("AquaSense");

  display.display();

  delay(3000);
}

void loop() {

  // =========================
  // TEMPERATURE
  // =========================
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);

  // =========================
  // PH SENSOR
  // =========================
  int phRaw = analogRead(phPin);

  voltagePH = phRaw * (5.0 / 1023.0);

  pHValue = 7 + ((2.5 - voltagePH) / 0.18);

  // =========================
  // TURBIDITY
  // =========================
  turbidityValue = analogRead(turbidityPin);

  turbidityNTU = map(turbidityValue, 0, 1023, 300, 0);

  // =========================
  // TDS SENSOR
  // =========================
  tdsValue = analogRead(tdsPin);

  float tdsVoltage = tdsValue * (5.0 / 1023.0);

  tdsPPM = (133.42 * tdsVoltage * tdsVoltage * tdsVoltage
           - 255.86 * tdsVoltage * tdsVoltage
           + 857.39 * tdsVoltage) * 0.5;

  // =========================
  // SERIAL MONITOR
  // =========================
  Serial.print("pH: ");
  Serial.print(pHValue);

  Serial.print(" | Turbidity: ");
  Serial.print(turbidityNTU);

  Serial.print(" NTU | TDS: ");
  Serial.print(tdsPPM);

  Serial.print(" ppm | Temp: ");
  Serial.println(temperatureC);

  // =========================
  // OLED DISPLAY
  // =========================
  display.clearDisplay();

  // NORMAL TEXT SIZE
  display.setTextSize(1);

  display.setCursor(0, 0);

  display.print("pH: ");
  display.println(pHValue, 1);

  display.print("Turb: ");
  display.print(turbidityNTU, 0);
  display.println(" NTU");

  display.print("TDS: ");
  display.print(tdsPPM, 0);
  display.println(" ppm");

  display.print("Temp: ");
  display.print(temperatureC, 1);
  display.println(" C");

  // =========================
  // WATER STATUS CONDITIONS
  // =========================

  bool safePH = (pHValue >= 6.5 && pHValue <= 8.5);
  bool safeTurbidity = (turbidityNTU <= 5);
  bool safeTDS = (tdsPPM <= 300);

  // SAFE
  if (safePH && safeTurbidity && safeTDS) {

    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);

    digitalWrite(buzzer, LOW);

    // PUMP OFF
    digitalWrite(relayPin, HIGH);

    display.println("STATUS: SAFE");
  }

  // WARNING
  else if (turbidityNTU <= 10 && tdsPPM <= 500) {

    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);

    // BEEP
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);

    // PUMP ON
    digitalWrite(relayPin, LOW);

    display.println("STATUS: WARNING");
    display.println("FILTERING...");
  }

  // UNSAFE
  else {

    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);

    // CONTINUOUS BUZZER
    digitalWrite(buzzer, HIGH);

    // PUMP ON
    digitalWrite(relayPin, LOW);

    display.println("STATUS: UNSAFE");
    display.println("FILTERING...");
  }

  display.display();

  delay(2000);
}