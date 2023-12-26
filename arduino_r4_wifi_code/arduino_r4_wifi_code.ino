/*
  Note: code is inspired from existing Arduino R4 WiFi examples
  
  Flow:
  - connect to the WiFi
  - Initialize Arduino UNO R4 LED Matrix
  - Connect to remote server and send an HTTPS GET request
  - read the response body and scroll the string on the LED matrix
*/

#include "WiFiS3.h";
#include "WiFiSSLClient.h";
#include "ArduinoGraphics.h";
#include "Arduino_LED_Matrix.h";

char ssid[] = "YOUR_WIFI_NAME";  // Change to your network SSID (name)
char pass[] = "YOUR_WIFI_PASS";  // Change to your network password 
int keyIndex = 0;             // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// The remote server you want to fetch data from (change to your domain)
char server[] = "arduino-production.up.railway.app"; 

WiFiSSLClient client;
ArduinoLEDMatrix matrix;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

   // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);  
    // wait 10 seconds for connection:
    delay(10000);
  }  
  printWifiStatus();

  // init LED matrix
  matrix.begin();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  // add some static text
  // will only show "UNO" (not enough space on the display)
  const char text[] = "UNO r4";
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText();
  matrix.endDraw();
  Serial.println("\nStarting connection to server...");
}

// function to scroll the text on the LED Matrix
void scroll_text(char text[]) {
  // Create a buffer to hold the modified text
    char modifiedText[strlen(text) + 11]; // 5 whitespaces at the start, 5 at the end, and 1 for null terminator

    // Add 5 whitespaces at the beginning
    strcpy(modifiedText, "     "); // 5 spaces
    // Append the original text
    strcat(modifiedText, text);
    // Add 5 whitespaces at the end
    strcat(modifiedText, "     "); // 5 spaces

    // Make it scroll!
    matrix.beginDraw();

    matrix.stroke(0xFFFFFFFF);
    matrix.textScrollSpeed(50);

    matrix.textFont(Font_5x7);
    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.println(modifiedText);
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();
}

// this function trys to read the response body (expects a simple string value)
// and scrolls the text on the LED matrix
void read_response() {
  String currentLine = ""; 
  boolean isHeader = true;
  String responseBody = "";
  boolean inBody = false;

  while (client.available()) {
    char c = client.read();

    if (c == '\n') {
      if (currentLine.length() == 0) {
        if (isHeader) {
          // Headers are complete, start reading the body
          inBody = true;
        }
        isHeader = false;
      } else {
        if (isHeader) {
          // Process headers if needed
        }
        currentLine = ""; // Reset current line
      }
    } else if (c != '\r') {
      currentLine += c; // Append character to current line
    }

    if (inBody && c != '\r' && c != '\n') {
      responseBody += c; // Accumulate the response body
    }
  }

  if (responseBody.length() > 0) {
    // Convert the responseBody to a character array and call scroll_text
    char bodyText[responseBody.length() + 1];
    responseBody.toCharArray(bodyText, sizeof(bodyText));
    // scroll the text on the LED Matrix
    scroll_text(bodyText);
  }
}

void loop() {
  Serial.println("\nSending GET request");

  // Connect over HTTPS
  if (client.connect(server, 443)) {
    Serial.println("connected to server");
    // Make the GET request
    client.println("GET /get-string HTTP/1.1");
    client.println("Host: arduino-production.up.railway.app"); // change to your host
    client.println("Connection: close");
    client.println();
    // wait for 2 seconds
    delay(2000);

    Serial.println("Trying to read response");
    read_response();
    client.flush();
    client.stop();
  }
  else {
    Serial.println("Server connection failed");
  }

  // wait for 5 seconds
  delay(5000);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
