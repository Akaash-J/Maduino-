#define DEBUG false

#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7

double latitude = 0.0;
double longitude = 0.0;
bool satellitesConnected = false;

String sendData(String command, const int timeout, boolean debug = false) {
  String response = "";
  Serial1.println(command);
  
  long int startTime = millis();
  while (millis() - startTime < timeout) {
    if (Serial1.available()) {
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

double convertToDecimalDegrees(String nmeaCoord) {
  double rawCoord = nmeaCoord.toDouble();
  int degrees = int(rawCoord / 100);
  double minutes = rawCoord - (degrees * 100);
  return degrees + (minutes / 60);
}

bool gpsLocation(String response) {
  int latStart = response.indexOf(':') + 2; 
  int latEnd = response.indexOf(',', latStart);
  String latString = response.substring(latStart, latEnd);

  int latDirStart = latEnd + 1;
  int latDirEnd = response.indexOf(',', latDirStart);
  String latDir = response.substring(latDirStart, latDirEnd);

  int lonStart = latDirEnd + 1;
  int lonEnd = response.indexOf(',', lonStart);
  String lonString = response.substring(lonStart, lonEnd);

  int lonDirStart = lonEnd + 1;
  int lonDirEnd = response.indexOf(',', lonDirStart);
  String lonDir = response.substring(lonDirStart, lonDirEnd);

  if (latString.length() > 0 && lonString.length() > 0) {
    double newLatitude = convertToDecimalDegrees(latString);
    if (latDir == "S") newLatitude = -newLatitude;

    double newLongitude = convertToDecimalDegrees(lonString);
    if (lonDir == "W") newLongitude = -newLongitude;

    if (newLatitude != 0.0 && newLongitude != 0.0) {
      latitude = newLatitude;
      longitude = newLongitude;

      SerialUSB.print("Latitude: ");
      SerialUSB.println(latitude, 6);

      SerialUSB.print("Longitude: ");
      SerialUSB.println(longitude, 6);

      if (!satellitesConnected) {
        SerialUSB.println("Connected to satellites");
        satellitesConnected = true;
      }
      return true;
    } else {
      SerialUSB.println("Invalid GPS data");
    }
  } else {
    SerialUSB.println("GPS data not available");
  }
  return false;
}

void sendSMS(String phoneNumber, String message) {
  sendData("AT+CMGF=1", 3000, DEBUG); 
  sendData("AT+CMGS=\"" + phoneNumber + "\"", 3000, DEBUG); 
  delay(1000);
  Serial1.print(message);
  delay(1000);
  Serial1.write(26);
  delay(5000);
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
  
  SerialUSB.println("Initializing GPS module...");

  String response = sendData("AT+CGPS=0", 3000, DEBUG);
  if (response.indexOf("OK") == -1) {
    SerialUSB.println("Error disabling GPS module");
    return;
  }
  
  delay(1000); 
  
  response = sendData("AT+CGPS=1", 3000, DEBUG);
  if (response.indexOf("OK") == -1) {
    SerialUSB.println("Error enabling GPS module");
    return;
  }
  
  delay(1000); 
  
}

void loop() {
  String gpsInfo = sendData("AT+CGPSINFO", 3000, DEBUG);
  if (gpsLocation(gpsInfo)) {
    String smsMessage = "Latitude: " + String(latitude, 6) + ", Longitude: " + String(longitude, 6);
    sendSMS("+918637447158", smsMessage);
  }
  
  delay(15000); // need 2 adjust for sms
}