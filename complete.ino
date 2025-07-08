#include <ps5Controller.h>

// Right motor pins
int enableRightMotor = 22;
int rightMotorPin1 = 16;
int rightMotorPin2 = 17;

// Left motor pins
int enableLeftMotor = 23;
int leftMotorPin1 = 18;
int leftMotorPin2 = 19;

// PWM configuration
const int PWMFreq = 1000; // 1 KHz
const int PWMResolution = 10; // 10-bit resolution (0–1023)
const int rightMotorPWMSpeedChannel = 4;
const int leftMotorPWMSpeedChannel = 5;

// Movement state
bool isMoving = false;  // false = stopped, true = active

void notify() {
  // Handle start and stop
  if (ps5.data.button.cross) {
    isMoving = true;
    Serial.println("Start button pressed.");
  }

  if (ps5.data.button.circle) {
    isMoving = false;
    rotateMotor(0, 0);
    Serial.println("Stop button pressed.");
  }

  if (!isMoving) return;  // Ignore joystick if stopped

  // Read stick values
  int yAxisValue = ps5.data.analog.stick.ly;  // Left stick Y-axis
  int xAxisValue = ps5.data.analog.stick.rx;  // Right stick X-axis

  int throttle = map(yAxisValue, -127, 127, -1023, 1023);
  int steering = map(xAxisValue, -127, 127, -1023, 1023);

  int motorDirection = throttle < 0 ? -1 : 1;

  int rightMotorSpeed = abs(throttle) - steering;
  int leftMotorSpeed = abs(throttle) + steering;

  rightMotorSpeed = constrain(rightMotorSpeed, 0, 1023);
  leftMotorSpeed = constrain(leftMotorSpeed, 0, 1023);

  rotateMotor(rightMotorSpeed * motorDirection, leftMotorSpeed * motorDirection);
}

void onConnect() {
  Serial.println("PS5 Controller Connected!");
}

void onDisConnect() {
  rotateMotor(0, 0);
  isMoving = false;
  Serial.println("PS5 Controller Disconnected!");
}

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed) {
  // Right motor direction
  if (rightMotorSpeed < 0) {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);
  } else if (rightMotorSpeed > 0) {
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);
  } else {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, LOW);
  }

  // Left motor direction
  if (leftMotorSpeed < 0) {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);
  } else if (leftMotorSpeed > 0) {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);
  } else {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, LOW);
  }

  // Apply PWM speed
  ledcWrite(rightMotorPWMSpeedChannel, abs(rightMotorSpeed));
  ledcWrite(leftMotorPWMSpeedChannel, abs(leftMotorSpeed));
}

void setUpPinModes() {
  pinMode(enableRightMotor, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);

  pinMode(enableLeftMotor, OUTPUT);
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);

  // Setup PWM channels
  ledcSetup(rightMotorPWMSpeedChannel, PWMFreq, PWMResolution);
  ledcSetup(leftMotorPWMSpeedChannel, PWMFreq, PWMResolution);

  // Attach PWM pins
  ledcAttachPin(enableRightMotor, rightMotorPWMSpeedChannel);
  ledcAttachPin(enableLeftMotor, leftMotorPWMSpeedChannel);

  // Stop motors on start
  rotateMotor(0, 0);
}

void setup() {
  setUpPinModes();
  Serial.begin(115200);

  ps5.attach(notify);
  ps5.attachOnConnect(onConnect);
  ps5.attachOnDisconnect(onDisConnect);
  ps5.begin("24:A6:FA:2F:A7:FC");  // Replace with your PS5 controller MAC

  while (!ps5.isConnected()) {
    Serial.println("Waiting for PS5 controller...");
    delay(300);
  }

  Serial.println("Controller connected. Press X to start, O to stop.");
}

void loop() {
  // All real-time control is handled in notify()
}
