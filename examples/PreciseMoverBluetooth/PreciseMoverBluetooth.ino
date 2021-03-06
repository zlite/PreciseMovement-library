#include <PreciseMover.h>
#include <SoftwareSerial.h>
#include <ArduinoBlue.h>

// Forward declarations needed for these methods.
void motorSetForward();
void motorSetCCW();
void motorSetCW();
void motorBrake();

// =======================================================================================================
// ========== ARDUINO PINS ===============================================================================
// =======================================================================================================

// REQUIRED: Set all the pins and verify them.

// Encoder pins
const int ENCODER_LEFT_PIN = 2;
const int ENCODER_RIGHT_PIN = 3;

// Motor one
const int ENA = 6;
const int IN1 = 5;
const int IN2 = 4;
// Motor two
const int ENB = 9;
const int IN3 = 10;
const int IN4 = 11;

// HM10 bluetooth module pins
const int BLUETOOTH_TX = 8;
const int BLUETOOTH_RX = 7;

// =======================================================================================================
// ========== ROBOT MEASUREMENTS =========================================================================
// =======================================================================================================

// You can select any length unit as long as all three below have same untis.
const double RADIUS = 29; // Radius of the wheel of the robot in mm
const double LENGTH = 104; // Distance from the left wheel to right wheel in mm

// =======================================================================================================
// ========== PID PARAMETERS =============================================================================
// =======================================================================================================

/* REQUIRED:
 * KP_FW: Keep incrementally increasing this until the robot is able to go straight.
 *        If the robot cannot go straight by tuning this, it is likely the issue is with the hardware. (ie. wheel alignment)
 * KP_TW: Increase this until the angular velocity of the twisting motion is fast enough to twist.
 *        Make sure that the twisting angular velocity is not too fast as this may lead to overshoot.
 *        Note that with just the KP_TW, the TARGET_TWIST_OMEGA will not be reached, but this is not necessary
 *        since we only need it to twist at a slow speed not at a specific speed.
 *        If you want it so that the TARGET_TWIST_OMEGA is precisely reached, you will need to tune the KI_TW as well.
 */

 // PID parameters for pure pursuit (path following and going straight)
const double KP_FW = 100;
const double KI_FW = 0;
const double KD_FW = 0;

// PID parameters for twisting
const int PID_TW_MIN = 100;
const int PID_TW_MAX = 255;
const double KP_TW = 5;
const double KI_TW = 0;
const double KD_TW = 0;

// =======================================================================================================
// ========== FORWARD AND TWISTING MOTION PARAMETERS =====================================================
// =======================================================================================================

// The motors will stop when it is this length from the target.
// This prevents overshoot.
const double STOP_LENGTH = 40; // mm

// Acceptable angle error after twisting.
const int ANGLE_ERROR_THRESHOLD = 3; // 3 degrees

// target PWM motor speed forward movement.
// This should be a slow enough for stability
const int TARGET_FORWARD_SPEED = 200; // PWM value 0 to 255

// target robot angular velocity during twisting motion.
const int TARGET_TWIST_MOTOR_OMEGA = 5; // in RPM

// Number of pulses of the encoder per revolution of wheel.
const int PULSES_PER_REV = 144;

// Look ahead parameter for pure pursuit algorithm.
// Don't worry about changing this unless you know what you're doing.
const double LOOK_AHEAD = 100;

// =======================================================================================================
// ========== VARIABLES ==================================================================================
// =======================================================================================================

// Keep track of encoder pulses
volatile unsigned long pulsesLeft = 0;
volatile unsigned long pulsesRight = 0;

// Previous pulse read times in microseconds
unsigned long prevLeftPulseReadTime = 0;
unsigned long prevRightPulseReadTime = 0;

const int MIN_MOTOR_SPEED = 100;

// PreciseMover object
PreciseMover mover(
	&pulsesLeft,
	&pulsesRight,
	PULSES_PER_REV,
	ENA,
	ENB,
	LENGTH,
	RADIUS,
	MIN_MOTOR_SPEED,
	TARGET_FORWARD_SPEED,
	STOP_LENGTH,
	TARGET_TWIST_MOTOR_OMEGA,
	ANGLE_ERROR_THRESHOLD,
	&motorSetForward,
	&motorSetCCW,
	&motorSetCW,
	&motorBrake,
	KP_FW,
	KI_FW,
	KP_TW,
	KI_TW);

SoftwareSerial softwareSerial(BLUETOOTH_TX, BLUETOOTH_RX);
ArduinoBlue bluetooth(softwareSerial);

// =======================================================================================================
// ========== FUNCTIONS ==================================================================================
// =======================================================================================================

/* REQUIRED:
  Change the motorBrake, motorSetForward, motorSetCW, and motorSetCCW functions as necessary.
  Then, test them one by one and verify they work properly. */

  // Configures the motor controller to stop the motors
void motorBrake() {
	digitalWrite(ENA, LOW);
	digitalWrite(ENB, LOW);
	digitalWrite(IN1, LOW);
	digitalWrite(IN2, LOW);
	digitalWrite(IN3, LOW);
	digitalWrite(IN4, LOW);
	digitalWrite(ENA, HIGH);
	digitalWrite(ENB, HIGH);
}

// Configures the motor controller to go forward.
void motorSetForward() {
	digitalWrite(IN1, HIGH);
	digitalWrite(IN2, LOW);
	digitalWrite(IN3, HIGH);
	digitalWrite(IN4, LOW);
}

// TODO: Configure dead reckoner!!!!!!!!!!!!!!!!!!
// Configures the motor controller to twist clockwise.
void motorSetCW() {
	digitalWrite(IN1, HIGH);
	digitalWrite(IN2, LOW);
	digitalWrite(IN3, LOW);
	digitalWrite(IN4, HIGH);
}

// Configures the motor controller to twist counter-clockwise.
void motorSetCCW() {
	digitalWrite(IN1, LOW);
	digitalWrite(IN2, HIGH);
	digitalWrite(IN3, HIGH);
	digitalWrite(IN4, LOW);
}

// Called whenever left pulse is detected
void pulseLeft() {
	// Only process the interrupt if it has been high for a minimum of 250 microseconds.
	if (digitalRead(ENCODER_LEFT_PIN) && micros() - prevLeftPulseReadTime > 250) {
		prevLeftPulseReadTime = micros();
		pulsesLeft++;
	}
}

// Called whenever right pulse is detected
void pulseRight() {
	// Only process the interrupt if it has been high for a minimum of 250 microseconds.
	if (digitalRead(ENCODER_RIGHT_PIN) && micros() - prevRightPulseReadTime > 250) {
		prevRightPulseReadTime = micros();
		pulsesRight++;
	}
}

// Attaches interrupts so that pulseLeft or pulseRight will be called whenever a pulse is detected
void attachInterrupts() {
	attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT_PIN), pulseLeft, HIGH);
	attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT_PIN), pulseRight, HIGH);
}

void setMotorPinModes() {
	pinMode(ENA, OUTPUT);
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(ENB, OUTPUT);
	pinMode(IN3, OUTPUT);
	pinMode(IN4, OUTPUT);
}

// =======================================================================================================
// ========== YOUR CODE HERE =============================================================================
// =======================================================================================================

/* RECOMMENDED:
  Use 115200 as the serial baud width.
  Make sure to set the baud rate accordingly on the Arduino serial monitor. */

void setup() {
	Serial.begin(115200);
	attachInterrupts();
	setMotorPinModes();
	Serial.println(" ------------------ SETUP COMPLETE ------------------");

	softwareSerial.begin(9600);

	//motorSetForward();
	//digitalWrite(ENA, HIGH);
	//digitalWrite(ENB, HIGH);
}

void loop() {
	//mover.tuneTwistPID();

	int button = bluetooth.getButton();

	if (button == 0) {
		// Straight for 1 meter
		mover.forward(1000);
	}
	else if (button == 1) {
		// 90 deg CCW
		mover.twist(90);
	}
	else if (button == 2) {
		// 90 deg CW
		mover.twist(-90);
	}
	else if (button == 3) {
		// 45 deg CCW
		mover.twist(45);
	}
	else if (button == 4) {
		// 45 deg CW
		mover.twist(-45);
	}

	/*Serial.print("left: "); Serial.print(pulsesLeft);
	Serial.print("\tright: "); Serial.println(pulsesRight);*/
}
