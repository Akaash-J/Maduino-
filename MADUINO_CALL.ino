#define DEBUG false

#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7
const int switch1Pin = 8;
#define PHONE_NUMBER "8637447158"


String sendData(String command, const int timeout, boolean debug = false) {
    String response = "";
    Serial1.println(command);

    long int startTime = millis();
    while (millis() - startTime < timeout) {
        while (Serial1.available()) {
            char c = Serial1.read();
            response += c;
        }
    }

    if (debug) {
        SerialUSB.print(command);
        SerialUSB.print(" Response: ");
        SerialUSB.println(response);
    }

    return response;
}

void setup() {
    SerialUSB.begin(115200);
    Serial1.begin(115200);

    pinMode(LTE_RESET_PIN, OUTPUT);
    digitalWrite(LTE_RESET_PIN, LOW);

    pinMode(LTE_PWRKEY_PIN, OUTPUT);
    digitalWrite(LTE_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(LTE_PWRKEY_PIN, HIGH);
    delay(2000);
    digitalWrite(LTE_PWRKEY_PIN, LOW);

    pinMode(LTE_FLIGHT_PIN, OUTPUT);
    digitalWrite(LTE_FLIGHT_PIN, LOW);

    pinMode(switch1Pin, INPUT_PULLUP);  // Enable internal pull-up resistor

    while (!SerialUSB) {
        delay(10);
    }

    SerialUSB.println("initializing....");
    //String response = sendData("AT+CFUN=1,1", 5000, DEBUG);
    String response = sendData("AT+CGATT=0", 2000, DEBUG);
   response = sendData("AT+CGATT=1", 2000, DEBUG);
  response = sendData("AT+CGACT=1,1", 2000, DEBUG);
  response = sendData("AT+CGPADDR=1", 1300, DEBUG);
  if (response.indexOf("OK") != -1 && response.indexOf(".") != -1) {
    SerialUSB.println("Internet connected.");
  } else {
    SerialUSB.println("Internet not connected.");
  }
    response = sendData("AT+CGPS=0", 3000, DEBUG);
    response = sendData("AT+CGPS=1", 3000, DEBUG);
}

void loop() {
    // Check if the switch is pressed (LOW means pressed due to pull-up)
    if (digitalRead(switch1Pin) == LOW) {
        SerialUSB.println("Switch pressed. Making call...");
        delay(300);  // Debounce the switch
        makeVoiceCall(PHONE_NUMBER);  // Make the call
    }

    delay(100);  // Small delay to avoid continuously reading the switch
}

// Function to make a voice call
void makeVoiceCall(String phoneNumber) {
    SerialUSB.println("Dialing number: " + phoneNumber);
  
    // Dial the number using ATD command
    String response = sendData("ATD" + phoneNumber + ";", 10000, DEBUG);
  
    // Check if call is initiated (look for "OK" or "CONNECT")
    if (response.indexOf("OK") != -1 || response.indexOf("CONNECT") != -1) {
        SerialUSB.println("Call initiated successfully.");
        monitorCall();
    } else {
        SerialUSB.println("Failed to initiate call. Response: " + response);
    }
}

void monitorCall() {
    SerialUSB.println("Monitoring call status...");

    bool callEnded = false;
    
    while (!callEnded) {
        // Read response from AT+CLCC command
        String response = sendData("AT+CLCC", 1300, DEBUG);  // Check call status
        
        // Check for the call ended status
        if (response.indexOf("NO CARRIER") != -1) {
            SerialUSB.println("Call ended.");
            callEnded = true;  // Exit the loop once the call has ended
        } else if (response.indexOf("ERROR") != -1) {
            SerialUSB.println("Error while monitoring call status.");
            callEnded = true;  // Exit the loop on error
        } else {
            // Continue monitoring if no end status is detected
            SerialUSB.println("Waiting for call end status...");
        }

        // Short delay to avoid flooding the module with commands
        delay(500);  // Check every half second
    }

    // Hang up the call
    String hangupResponse = sendData("ATH", 1300, DEBUG);
    if (hangupResponse.indexOf("OK") != -1) {
        SerialUSB.println("Call hung up successfully.");
    } else {
        SerialUSB.println("Failed to hang up call. Response: " + hangupResponse);
    }
}
