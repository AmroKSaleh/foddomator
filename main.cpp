/*
   @author: Foddomator Team
   @date: 16-2-2022
   @version: 1.1
*/

/* ----------------------------------- /
   Fake Test Schedule:                 /
   ----------------------------------- /
   - Each 30 sec feed 1 Cow            /
   - Test water level cont.            /
   - Turn on the water when needed pumb/
   - Test food level cont.             /
   - Turn on a food level warning LED  /
   if the level gets too low           /
   ----------------------------------- /
*/

// Include the RTC module DS3231.h library
#include <DS3231.h>

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

// Time Variables
String now;
char oldNow = "";

// Water Pumb Pins
const int pumbPin     = 4;

// Water Level Sensor Pins
const int sensorPower = 7;
const int sensorPin   = A0;

// Water Level Variables
int       waterLevel        = 0;
int       level             = 0;
const int waterRefillLevel  = 180;
const int waterFullLevel    = 320;

// HC-04 Pins
const int echoPin     = 2;
const int trigPin     = 3;

// Food Level Variables
const float containerH  = 13.5;
const int warningLEDPin = 5;
long duration;
long currentFoodLevel;
const int minFoodLevel  = 5;

// Food Dispenser Motor
int EnA = 9;
int IN1 = 8;
int IN2 = 7;

void setup()
{
  pinMode(pumbPin, OUTPUT); // Sets the pumbPin as an OUTPUT
  pinMode(sensorPower, OUTPUT); // Sets the sensorPower as an OUTPUT
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  pinMode(warningLEDPin, OUTPUT); // Sets the warningLEDPin as an OUTPUT

  // Set motor control pins as OUTPUT
  pinMode(EnA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Set sensorPower to LOW so no power flows through
  // the sensor to save power
  digitalWrite(sensorPower, LOW);
  // Set the warningLEDPin to LOW
  digitalWrite(warningLEDPin, LOW);

  // Initialize the rtc object
  rtc.begin();

  // Initialize Serial Communication
  Serial.begin(9600);

  now = currentTime();
  oldNow = now[4];
}

void loop()
{
  waterLevelCheck();
  Serial.println("Water Level Checked");

  foodLevelCheck();
  Serial.println("Food Level Checked");

  if (now[4] != oldNow)
  {
    // Dispense food sequence
    digitalWrite(IN1, HIGH); // Turn on the motor
    digitalWrite(IN2, LOW);

    // Set speed to 200
    analogWrite(EnA, 255);

    Serial.println("Dispensing Food...");
    //    oldNow = now[4];
    delay(4000);

    // Turn off the motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(EnA, 0);
    Serial.println("Done Dispensing Food...");
  }

  now = currentTime();

  Serial.println("-------------------------------");
  delay(1000);
}

/*
   @returns: String readTime the current time reading
   @version: 1.0

   uses the RTC module to return current time in 24:00:00
   String format
*/
String currentTime()
{
  // Send time
  String readTime = rtc.getTimeStr();
  Serial.println(now);
  return readTime;
}

/*
   @version: 1.0

   uses foodLevel to get the food level current value
   and prints it in the serial moniter for debuging and
   turns on/off the warning LED according to the food
   level reading value
*/
void foodLevelCheck()
{
  int currentLevel = foodLevel();
  if (currentLevel < minFoodLevel)
  {
    digitalWrite(warningLEDPin, HIGH);
    Serial.println("Food Warning Triggered");
  }
  else {
    digitalWrite(warningLEDPin, LOW);
  }
}

/*
   @returns: int currentFoodLevel the amount of remaining
   food in the container
   @version: 1.0

   uses the HC-04 sensor to calculate the amount of left food
   in the container
*/
int foodLevel()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  // to send an ultrasonic wave
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  currentFoodLevel = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // calculate the food level by subtracting the empty part
  // from the total container height
  currentFoodLevel = containerH - currentFoodLevel;

  Serial.print("Food Level: ");
  Serial.print(currentFoodLevel);
  Serial.println(" cm");
  return currentFoodLevel;
}

/*
   @version: 1.0

   uses readWaterSensor to get the water level current value
   and prints it in the serial moniter for debuging and
   turns on/off the water pumb according to the water level
   reading value
*/
void waterLevelCheck()
{
  //get the water level
  waterLevel = readWaterSensor(sensorPin, sensorPower);

  Serial.print("Water level: ");
  Serial.println(level);

  if (waterLevel <= waterRefillLevel) {
    while (waterLevel < waterFullLevel)
    {
      digitalWrite(pumbPin, HIGH);
      Serial.print("Water Level: ");
      Serial.println(waterLevel);
      waterLevel = readWaterSensor(sensorPin, sensorPower);
    }
    digitalWrite(pumbPin, LOW);
  }
}

/*
   @param: int Pin the sensor pin to be readen
   @param: int Power the sensor power pin to save power while not reading
   @returns: int waterLevel the result of reading water level value
   @version: 1.0

   powers on the sensor and reads the value and returns
   the water level reading
*/
int readWaterSensor(int Pin, int Power) {
  digitalWrite(Power, HIGH);  // Turn the sensor ON
  delay(10);                  // wait 10 milliseconds
  level = analogRead(Pin);      // Read the analog value form sensor
  digitalWrite(Power, LOW);   // Turn the sensor OFF
  return level;                 // send current reading
}
