const int switch1Pin = 8;
const int switch2Pin = 9;
const unsigned long pressDuration1 = 500; // 500 milliseconds for switch 1
const unsigned long pressDuration2 = 2000; // 2 seconds for switch 2

bool switch1State = LOW;
bool switch2State = LOW;
bool switch1LongPressDetected = false;
bool switch2LongPressDetected = false;
unsigned long switch1PressedTime = 0;
unsigned long switch2PressedTime = 0;

void setup() {
  // Initialize the serial monitor for Maduino Zero 4G LTE
  SerialUSB.begin(115200);

  // Set up the switch pins
  pinMode(switch1Pin, INPUT_PULLUP); // Enable internal pull-up resistor
  pinMode(switch2Pin, INPUT_PULLUP); // Enable internal pull-up resistor

  // Give some time for the serial monitor to initialize
  while (!SerialUSB) {
    delay(10);
  }

  SerialUSB.println("Switch test initialized.");
}

void loop() {
  // Read the state of the switches
  bool switch1Reading = digitalRead(switch1Pin) == LOW; // LOW when pressed
  bool switch2Reading = digitalRead(switch2Pin) == LOW; // LOW when pressed

  // Handle switch 1 press duration
  if (switch1Reading) {
    if (!switch1State) {
      switch1State = true;
      switch1PressedTime = millis();
      switch1LongPressDetected = false;
    } else {
      if (!switch1LongPressDetected && millis() - switch1PressedTime >= pressDuration1) {
        SerialUSB.println("SOS");
        switch1LongPressDetected = true; // Prevent further "SOS" prints until released
        // Optionally, you can add HTTP request here if needed
      }
    }
  } else {
    switch1State = false;
    switch1LongPressDetected = false;
  }

  // Handle switch 2 press duration
  if (switch2Reading) {
    if (!switch2State) {
      switch2State = true;
      switch2PressedTime = millis();
      switch2LongPressDetected = false;
    } else {
      if (!switch2LongPressDetected && millis() - switch2PressedTime >= pressDuration2) {
        SerialUSB.println("OK");
        switch2LongPressDetected = true; // Prevent further "OK" prints until released
        // Optionally, you can add HTTP request here if needed
      }
    }
  } else {
    switch2State = false;
    switch2LongPressDetected = false;
  }
}
