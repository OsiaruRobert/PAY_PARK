#include <WiFi.h>
#include "ESP32HTTPClient.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>  // Included for I2C communication
#include <ESP32Servo.h>
#include <aWOT.h>
#include <ArduinoJson.h>

//Library setup
Servo myservo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP32HTTPClient client("https://pay-park-api.onrender.com");
WiFiServer server(80);
App app;



//variables
const int next_btn = 13;
const int back_btn = 14;

int next_btn_state = HIGH;
int back_btn_state = HIGH;
int last_next_state = HIGH;
int last_back_state = HIGH;

const int buzzer = 26;

bool can_click_next = true;
bool can_click_back = false;

String current_state = "start";
String current_message_up = "Welcome To Pay Park";
String current_message_down = "Press start to continue";

String reference = "";
String account_number = "";
String account_name = "";
String account_bank = "";

String chargeMessage = "";

String park_space = "";

// GLOBAL MARQUEE SETTINGS
String spacing = "                ";  // 16 spaces
String scrollMessage = spacing + "Welcome to Pay Park! Please tap next to start.";
int startIndex = 0;
unsigned long lastScrollTime = 0;
int scrollSpeedMs = 300;  // FIXED: Removed const so speed can be changed dynamically

//Internet
const char* API_KEY = "change-this-to-something-long";
const char* WIFI_SSID = "OSIARU";
const char* WIFI_PASS = "11111110";

// TIMING HELPERS FOR POLLING
unsigned long lastPollTime = 0;
const unsigned long pollIntervalMs = 5000;  // Poll backend every 5 seconds

// Forward declarations so the compiler can trace functions cleanly
void start_func();
void charge_func();
void display_aza_func();
void monitorpayment_func();
void thanks_func();
void updateScrollMessage(String text, int UscrollSpeedMs = 300);

// 1. Define the structure for each state object
struct StateNode {
  const char* state;
  const char* message;
  const char* previous_state;
  const char* next_state;
};

StateNode states[] = {
  { "start", "Welcome To Pay Park.", "", "charge" },
  { "charge", "Charging...", "start", "display_aza" },
  { "display_aza", "Please pay #100 to the account below.", "start", "thanks" },
  { "thanks", "Thanks for Parking with us.", "start", "start" }
};

void beep(int times = 1, int durationMs = 100) {
  for (int i = 0; i < times; i++) {
    digitalWrite(buzzer, HIGH);
    delay(durationMs);
    digitalWrite(buzzer, LOW);
    if (i < times - 1) delay(80);
  }
}

// Helper to easily change the running message from anywhere in your code
void updateScrollMessage(String text, int UscrollSpeedMs) {
  scrollSpeedMs = UscrollSpeedMs;  // FIXED: Safely updates the global speed engine variable
  scrollMessage = spacing + text;
  startIndex = 0;  // Reset index so new text scrolls cleanly from right
}

// 2. Handle button clicks per state
void buttonClicked(int buttonPressed) {
  // Run action based on the current state
  if (current_state == "start") {
    if (buttonPressed == next_btn) {
      Serial.println("Start -> Charge");
      current_state = "charge";
      beep(2, 60);
      charge_func();
    }
  } else if (current_state == "charge") {
    // Ignore button presses while charging
    Serial.println("Busy charging, please wait...");
    if (buttonPressed == back_btn) {
      Serial.println("Payment cancelled -> Start");
      beep(1, 60);
      start_func();
    }
  } else if (current_state == "display_aza") {
    if (buttonPressed == next_btn) {
      Serial.println("Manually checking payment status...");
      monitorpayment_func();  // Allow user to force manual validation check
    } else if (buttonPressed == back_btn) {
      Serial.println("Payment cancelled -> Start");
      beep(1, 60);
      start_func();
    }
  } else if (current_state == "thanks") {
    Serial.println("Thanks -> Start");
    start_func();
  }
}


// Returns true if the key is correct. Otherwise replies 401 and returns false.
bool checkKey(Request& req, Response& res) {
  // char* key = req.get("X-API-Key");
  // if (key == NULL || strcmp(key, API_KEY) != 0) {
  //   res.sendStatus(401);
  //   return false;
  // }
  return true;
}


// GET /status
void getStatus(Request& req, Response& res) {
  if (!checkKey(req, res)) return;
  res.set("Content-Type", "application/json");
  res.print("{\"status\":\"ok\",\"uptime\":");
  res.print((int)(millis() / 1000));
  res.print("}");
}

// POST /PAID { park_space}
// POST /led   body: {"state":"on"}  or  {"state":"off"}
void postPaid(Request& req, Response& res) {
  if (!checkKey(req, res)) return;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, req);

  res.set("Content-Type", "application/json");
  if (err) {
    res.status(400);
    res.print("{\"error\":\"invalid json\"}");
    return;
  }

  const char* return_park_space = doc["park_space"] | "";  // "" if missing
  park_space = return_park_space;
  thanks_func();

  res.status(201);
  res.print("{\"message\":\"Payment recieved\"}");
}


void setup() {
  Wire.begin(21, 22);  // Initialize I2C communication on pins (SDA=21, SCL=22)

  Serial.begin(115200);
  Serial.println("Booting System");

  pinMode(next_btn, INPUT_PULLUP);
  pinMode(back_btn, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);

  lcd.init();       // Initialize the LCD
  lcd.backlight();  // Turn on the backlight

  // Print static text on row 0 immediately
  lcd.setCursor(0, 0);
  lcd.print("  PAY PARK  ");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

 Serial.println(WiFi.localIP());

  app.get("/status", &getStatus);
  app.post("/paid", &postPaid);

  server.begin();

  client.setInsecure();  // Bypass local SSL certificate check for ngrok
  myservo.attach(4);
  myservo.write(0);  // Close gate

  // Call start function to cleanly set up the initial display state text
  start_func();
}

//==================state Functions==================//
void start_func() {
  beep(3);
  current_state = "start";
  can_click_next = true;
  can_click_back = false;
  reference = "";
  myservo.write(0);  // Close gate


  lcd.setCursor(0, 0);
  lcd.print("    Pay Park!  ");
  updateScrollMessage("Welcome to Pay Park... Please pay 100 to park... Press next to continue.");
}

void charge_func() {
  lcd.setCursor(0, 0);
  lcd.print("   PROCESSING   ");
  updateScrollMessage("Requesting billing profile... Please wait.");

  client.post("/charge/")
    .onError([](int code, const char* msg) {
      Serial.printf("FETCH Error %d: %s\n", code, msg);
    })
    .getBody("reference", &reference)
    .getBody("account_number", &account_number)
    .getBody("account_name", &account_name)
    .getBody("bank_name", &account_bank);

  if (client.isSuccess() && reference != "") {
    Serial.println("Billing Profile Received Successfully!");
    Serial.printf("Ref: %s, Acct: %s, Bank: %s\n", reference.c_str(), account_number.c_str(), account_bank.c_str());

    beep(2);

    // Transition to display state
    current_state = "display_aza";
    display_aza_func();
    lastPollTime = millis();  // Reset polling window entry timestamp
  } else {
    Serial.println("Failed to fetch billing profiles. Retrying...");
    lcd.setCursor(0, 0);
    lcd.print("  SYSTEM ERROR  ");

    delay(2000);
    start_func();
  }
}

void display_aza_func() {
  lcd.setCursor(0, 0);
  lcd.print("Pls pay #100 to");
  // chargeMessage = "Please pay #100 =>" + account_number + " " + account_bank + " (" + account_name + ")";
  chargeMessage = "  " + account_number + " " + account_bank + " #100.";
  updateScrollMessage(chargeMessage, 400);
}

// Performs non-blocking background polling requests against Node backend
void monitorpayment_func() {
  Serial.println("Polling payment validation endpoint...");

  bool isPaid = false;
  String path = "/paid/" + reference;

  client.get(path.c_str())
    .onError([](int code, const char* msg) {
      Serial.printf("Verification Error %d: %s\n", code, msg);
    })
    .getBody("paid", &isPaid)
    .getBody("park_space_u", &park_space);

  if (client.isSuccess() && isPaid == true) {
    Serial.println("Payment verified! Transitioning to thank you screen.");
    current_state = "thanks";
    thanks_func();
  } else {
    Serial.println("Payment not received yet.");
    // Restore text if changed by external error logs
    lcd.setCursor(0, 0);
    lcd.print("PENDING PAYMENT ");
  }
}

void thanks_func() {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("    SUCCESS     ");

  lcd.setCursor(0, 1);
  lcd.print("PARK IN : " + park_space);

  beep(3, 150);
  //problem here



  myservo.write(0);  // Open gate
  delay(2000);        // Freeze everything for 2 seconds while vehicle moves
  myservo.write(90);   // Close gate

  start_func();  // CRITICAL FIX: Automatically goes back to start state
}



void loop() {

  WiFiClient clientw = server.available();
  if (clientw.connected()) {
    app.process(&clientw);
  }

  next_btn_state = digitalRead(next_btn);
  back_btn_state = digitalRead(back_btn);

  // Trigger only on transition (HIGH to LOW)
  if (next_btn_state == LOW && last_next_state == HIGH) {
    Serial.println("The next_btn is pressed");
    buttonClicked(next_btn);
    delay(50);  // Small debounce
  }

  if (back_btn_state == LOW && last_back_state == HIGH) {
    Serial.println("The back_btn is pressed");
    buttonClicked(back_btn);
    delay(50);  // Small debounce
  }

  last_next_state = next_btn_state;
  last_back_state = back_btn_state;

  // // AUTOMATED NON-BLOCKING BACKGROUND POLLING TIMER
  // if (current_state == "display_aza") {
  //   if (millis() - lastPollTime >= pollIntervalMs) {
  //     lastPollTime = millis();
  //     monitorpayment_func();  // Hit node route directly
  //   }
  // }

  // GLOBAL NON-BLOCKING MARQUEE ENGINE (Runs on Row 1)
  if (millis() - lastScrollTime >= scrollSpeedMs) {
    lastScrollTime = millis();

    int messageLength = scrollMessage.length();
    String displaySubstring = scrollMessage.substring(startIndex, startIndex + 16);

    while (displaySubstring.length() < 16) {
      displaySubstring += " ";
    }

    lcd.setCursor(0, 1);
    lcd.print(displaySubstring);

    startIndex++;
    if (startIndex >= messageLength) {
      startIndex = 0;  // Reset marquee string back around
    }
  }
}
