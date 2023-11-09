#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const int OUTPUT_PIN = 1;

// WiFI credentials
const char *ssid = "";
const char *password = "";

// API credentials 
const char *apiKey = "";
const char *apiURL = "https://legend.lnbits.com/api/v1/payments?limit=1";

// How often would you like to check for new payments?
const unsigned long checkInterval = 5000;

String newestInvoiceId = "";                  
unsigned long previousMillis = 0;

String getNewestPaidInvoiceId()
{
  HTTPClient http;

  if (http.begin(apiURL))
  {
    http.addHeader("X-Api-Key", apiKey);

    int httpCode = http.GET();

    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();

        DynamicJsonDocument jsonDoc(2048); // Adjust the size as needed
        DeserializationError error = deserializeJson(jsonDoc, payload);

        if (!error)
        {
          JsonArray root = jsonDoc.as<JsonArray>();
          if (root.size() > 0)
          {
            JsonObject invoice = root[0];
            return invoice["checking_id"].as<String>();
          }
        }
        else
        {
          Serial.println("Failed to parse JSON");
        }
      }
      else
      {
        Serial.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    }
    else
    {
      Serial.println("Failed to connect to server");
    }
  }
  else
  {
    Serial.println("Failed to begin HTTP connection");
  }

  return ""; // Return an empty string if no invoice is found or an error occurs
}

void connectToWiFi()
{
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("🔌 Connected to WiFi");
}

void setup()
{
  pinMode(OUTPUT_PIN, OUTPUT);
  Serial.begin(115200);

  connectToWiFi();
  
  delay(1000);

  newestInvoiceId = getNewestPaidInvoiceId();

  Serial.println("✅ Setup finished");
  Serial.println("🆕 Newest invoice: ");
  Serial.println(newestInvoiceId);
}

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < checkInterval)
  {
    return;
  }
  
  previousMillis = currentMillis;

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi lost. Reconnecting...");
    WiFi.reconnect();
  }
  else
  {
    String currentInvoiceId = getNewestPaidInvoiceId();
    if(currentInvoiceId != "" && newestInvoiceId != "" && currentInvoiceId != newestInvoiceId) {
      Serial.println("✅ A new invoice has been paid!");
      Serial.println(currentInvoiceId);
      Serial.println(newestInvoiceId);

      newestInvoiceId = currentInvoiceId;

      Serial.println("🪙 is dispensed");

      digitalWrite(OUTPUT_PIN, HIGH);
      delay(100);
      digitalWrite(OUTPUT_PIN, LOW);
    }
    else {
      Serial.println("❌ No new invoices found");
    }
  }
}
