#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// =================================================
// RFID + SERVO 1
// =================================================
#define RFID_SS 10
#define RFID_RST 9
#define RFID_SERVO_PIN 6

MFRC522 rfid(RFID_SS, RFID_RST);
Servo rfidServo;

bool rfidServoActive = false;
unsigned long rfidServoStartTime = 0;


// =================================================
// ULTRASONIC + LED
// =================================================
#define TRIG_PIN 4
#define ECHO_PIN 5
#define LED_PIN 3

bool ledState = false;
bool ultrasonicDetected = false;


// =================================================
// RAIN SENSOR + SERVO 2
// =================================================
#define RAIN_PIN A0
#define RAIN_SERVO_PIN 7

Servo rainServo;

#define RAIN_THRESHOLD 500

bool rainDetected = false;


// =================================================
// IR SENSOR + SERVO 3
// =================================================
#define IR_PIN A1
#define IR_SERVO_PIN 8

Servo irServo;


// =================================================
// FLAME SENSOR + BUZZER
// =================================================
#define FLAME_PIN A2
#define BUZZER_PIN 2


// =================================================
// SETUP
// =================================================
void setup()
{
  Serial.begin(9600);

  // =================================================
  // RFID
  // =================================================
  SPI.begin();
  rfid.PCD_Init();

  rfidServo.attach(RFID_SERVO_PIN);
  rfidServo.write(0);


  // =================================================
  // ULTRASONIC
  // =================================================
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);


  // =================================================
  // RAIN SENSOR
  // =================================================
  pinMode(RAIN_PIN, INPUT);

  rainServo.attach(RAIN_SERVO_PIN);
  rainServo.write(0);


  // =================================================
  // IR SENSOR
  // =================================================
  pinMode(IR_PIN, INPUT);

  irServo.attach(IR_SERVO_PIN);
  irServo.write(0);


  // =================================================
  // FLAME SENSOR + BUZZER
  // =================================================
  pinMode(FLAME_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);


  // =================================================
  // SYSTEM READY
  // =================================================
  Serial.println("================================");
  Serial.println(" SMART HOME SYSTEM READY");
  Serial.println(" ALL SERVOS SET TO 0 DEGREES");
  Serial.println("================================");
}


// =================================================
// MAIN LOOP
// =================================================
void loop()
{

  // =================================================
  // 1. RFID -> SERVO 1
  // =================================================

  // Check whether RFID servo is currently active
  if (rfidServoActive)
  {
    // After 3 seconds, return servo to 0 degrees
    if (millis() - rfidServoStartTime >= 3000)
    {
      Serial.println("3 SECONDS COMPLETED");
      Serial.println("RFID SERVO RETURNING TO 0 DEGREES");

      rfidServo.write(0);

      rfidServoActive = false;

      // Allow RFID reader to detect again
      delay(300);
    }
  }
  else
  {
    // RFID can detect a new card
    if (rfid.PICC_IsNewCardPresent() &&
        rfid.PICC_ReadCardSerial())
    {
      Serial.println("--------------------------------");
      Serial.println("RFID CARD DETECTED");
      Serial.println("RFID SERVO ROTATING TO 90 DEGREES");

      // Servo 1 -> 90 degrees
      rfidServo.write(90);

      // Start 3-second timer
      rfidServoStartTime = millis();

      rfidServoActive = true;

      // Stop communication with the card
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

      Serial.println("RFID SERVO WILL RETURN AFTER 3 SECONDS");
      Serial.println("--------------------------------");
    }
  }


  // =================================================
  // 2. ULTRASONIC -> LED TOGGLE
  // =================================================

  long duration;
  float distance;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration > 0)
  {
    distance = duration * 0.0343 / 2;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance <= 20 && !ultrasonicDetected)
    {
      ultrasonicDetected = true;

      ledState = !ledState;

      digitalWrite(LED_PIN, ledState);

      if (ledState)
        Serial.println("LED ON");
      else
        Serial.println("LED OFF");
    }

    if (distance > 25)
    {
      ultrasonicDetected = false;
    }
  }


  // =================================================
  // 3. RAIN SENSOR -> SERVO 2
  // =================================================

  int rainValue = analogRead(RAIN_PIN);

  Serial.print("Rain Value: ");
  Serial.println(rainValue);

  if (rainValue < RAIN_THRESHOLD && !rainDetected)
  {
    rainDetected = true;

    Serial.println("RAIN DETECTED");
    Serial.println("RAIN SERVO ROTATING TO 90 DEGREES");

    rainServo.write(90);
  }

  if (rainValue < RAIN_THRESHOLD && rainDetected)
  {
    rainServo.write(90);
  }

  if (rainValue >= RAIN_THRESHOLD)
  {
    if (rainDetected)
    {
      Serial.println("RAIN STOPPED");
      Serial.println("RAIN SERVO REMAINS AT 90 DEGREES");

      rainServo.write(90);

      rainDetected = false;
    }
  }


  // =================================================
  // 4. IR SENSOR -> SERVO 3
  // =================================================

  if (digitalRead(IR_PIN) == LOW)
  {
    Serial.println("IR DETECTED");
    Serial.println("IR SERVO ROTATING TO 90 DEGREES");

    irServo.write(90);

    delay(7000);

    Serial.println("IR SERVO RETURNING TO 0 DEGREES");

    irServo.write(0);

    delay(500);
  }


  // =================================================
  // 5. FLAME SENSOR -> BUZZER
  // =================================================

  if (digitalRead(FLAME_PIN) == LOW)
  {
    Serial.println("FIRE DETECTED!");
    Serial.println("BUZZER ON");

    digitalWrite(BUZZER_PIN, HIGH);
  }
  else
  {
    Serial.println("NO FIRE");
    Serial.println("BUZZER OFF");

    digitalWrite(BUZZER_PIN, LOW);
  }


  // =================================================
  // SMALL LOOP DELAY
  // =================================================

  delay(100);
}