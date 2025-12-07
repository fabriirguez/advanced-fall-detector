#define ESP_DRD_USE_SPIFFS true

// ----------------------------
// Librerías estándar: ya instaladas si tienes el ESP32 configurado
// ----------------------------

#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>

// ----------------------------
// Librerías adicionales: deben instalarse desde el gestor
// ----------------------------

#include <WiFiManager.h>
// Portal cautivo para configurar la WiFi

// Instalable desde el gestor (buscar "WifiManager", versión Alpha)
// https://github.com/tzapu/WiFiManager

#include <ESP_DoubleResetDetector.h>
// Librería para detectar doble pulsación del botón de reset
// Útil para habilitar el modo de configuración
// Instalable desde el gestor (buscar "ESP_DoubleResetDetector")
// https://github.com/khoih-prog/ESP_DoubleResetDetector

#include <ArduinoJson.h>
// ArduinoJson se usa para parsear y crear el archivo de configuración
// Buscar "Arduino Json" en el gestor de librerías
// https://github.com/bblanchon/ArduinoJson

// -------------------------------------
// -------   Otras configuraciones   ------
// -------------------------------------

#include <Wire.h>
#include <MPU6050.h>
#define BUZZER_PIN 14
#define BUZZER_CHANNEL 0
#define BEEP_DURATION 5000 // 5 segundos (en milisegundos)
#include <HTTPClient.h>
#include "time.h"

const int PIN_LED = 2;

#define JSON_CONFIG_FILE "/sample_config.json"

// Número de segundos tras un reset durante los cuales
// un segundo reset se considerará doble reset
#define DRD_TIMEOUT 10

// Dirección de memoria RTC que usará el DoubleResetDetector
#define DRD_ADDRESS 0

// -----------------------------

// -----------------------------

DoubleResetDetector *drd;

// Bandera para guardar configuración
bool shouldSaveConfig = false;

char testString[50] = "deafult value";
unsigned long long testNumber = 12345678123ULL;
char testNumberStr[20]; // Buffer para almacenar el número convertido a cadena
int apikey = 1234567;

unsigned long button_time = 0;  
unsigned long last_button_time = 0;
const float fallThreshold = 650000; // Ajusta este valor según tus necesidades (m/s^3)
const int sampleInterval = 10;   // Intervalo en milisegundos entre lecturas
float prevAccX = 0.0, prevAccY = 0.0, prevAccZ = 0.0;
String serverName = "https://api.callmebot.com/whatsapp.php?";
const char* ntpServer = "in.pool.ntp.org";
const long  gmtOffset_sec = 106200;
const int   daylightOffset_sec = 0;
char dateTimeStr[30];
bool buttonInterrupted = false;
unsigned long delayStartTime = 0;
const int DOUBLE_PRESS_THRESHOLD = 1500; // Umbral temporal de doble pulsación (ms)
bool lastFallDetection = false;
unsigned long lastFallTime = 0;
IPAddress staticIP(192, 168, 1, 100); // Sustituye por la IP estática deseada
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8); // Sustituye por la IP de tu servidor DNS


struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

Button button1 = {12, 0, false};

MPU6050 mpu;

String urlEncode(const char* str) {
  const char* hex = "0123456789ABCDEF";
  String encodedStr = "";

  while (*str != 0) {
    if (('a' <= *str && *str <= 'z')
        || ('A' <= *str && *str <= 'Z')
        || ('0' <= *str && *str <= '9')) {
      encodedStr += *str;
    } else {
      encodedStr += '%';
      encodedStr += hex[*str >> 4];
      encodedStr += hex[*str & 0xf];
    }
    str++;
  }

  return encodedStr;
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  
  // Imprime solo el día y la hora actuales
  
  strftime(dateTimeStr, sizeof(dateTimeStr), "%A, %B %d %Y %H:%M:%S", &timeinfo);
}


void IRAM_ATTR isr() {
  button_time = millis();
  if (button_time - last_button_time > 250){
    button1.numberKeyPresses++;
    button1.pressed = true;
    last_button_time = button_time;
    buttonInterrupted = true; // Marca que hubo pulsación del botón
  }
}

void saveConfigFile()
{
  Serial.println(F("Saving config"));
  StaticJsonDocument<512> json;
  json["testString"] = testString;
  json["testNumber"] = testNumber;
  json["apikey"] = apikey;

  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile)
  {
    Serial.println("failed to open config file for writing");
  }

  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    Serial.println(F("Failed to write to file"));
  }
  configFile.close();
}

bool loadConfigFile()
{
  // Limpieza del FS (solo pruebas)
  // SPIFFS.format();

  // Leer configuración desde FS en formato JSON
  Serial.println("montando FS...");

  // Puede requerir begin(true) la primera vez que uses SPIFFS
  // Nota: begin(true) reformatea SPIFFS; solo debería llamarse si falla el montaje
  // Evítalo si tienes datos importantes que no deban perderse
  if (SPIFFS.begin(false) || SPIFFS.begin(true))
  {
    Serial.println("FS montado");
    if (SPIFFS.exists(JSON_CONFIG_FILE))
    {
      // El archivo existe: leyendo y cargando
      Serial.println("leyendo archivo de configuración");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if (configFile)
      {
        Serial.println("archivo de configuración abierto");
        StaticJsonDocument<512> json;
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error)
        {
          Serial.println("\nJSON parseado");

          strcpy(testString, json["testString"]);
          testNumber = json["testNumber"].as<unsigned long long>(); // Lectura correcta de testNumber
          apikey = json["apikey"].as<int>(); // Lectura correcta de apikey

          return true;
        }
        else
        {
          Serial.println("no se pudo cargar el JSON de configuración");
        }
      }
    }
  }
  else
  {
    Serial.println("fallo al montar FS");
  }
  // fin de lectura
  return false;
}





// Callback que indica que es necesario guardar la configuración
void saveConfigCallback()
{
  Serial.println("Se debe guardar la configuración");
  shouldSaveConfig = true;
}

// Se llama cuando se lanza el modo de configuración;
// útil para mostrar esta información en un display
void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Modo de configuración iniciado");

  Serial.print("Config SSID: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());

  Serial.print("Config IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void setup()
{

  Wire.begin(2, 15); // SDA a GPIO 22, SCL a GPIO 21

  // Inicializar MPU6050
  mpu.initialize();

  // Verificar la conexión
  Serial.println(mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);
  ledcSetup(BUZZER_CHANNEL, 2000, 16); // 2000 Hz, resolución de 16 bits
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  pinMode(PIN_LED, OUTPUT);

  bool forceConfig = false;

  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (drd->detectDoubleReset())
  {
    Serial.println(F("Forcing config mode as there was a Double reset detected"));
    forceConfig = true;
  }

  bool spiffsSetup = loadConfigFile();
  if (!spiffsSetup) {
    Serial.println(F("Forcing config mode as there is no saved config"));
    forceConfig = true;
  }

  //WiFi.disconnect();
  WiFi.mode(WIFI_STA); // Fijar modo explícitamente; por defecto STA+AP
  Serial.begin(115200);
  delay(10);

  // wm.resetSettings(); // borrar ajustes

  const char *configInfoText = "<div style='margin-bottom: 20px;'>Ask your emergency contact to open scan this QR <a href='https://i.ibb.co/rMzYN3z/callmebot-qr.png'>here</a> and provide their API key.</div>";

  // Texto a mostrar debajo de los parámetros
  const char *configInfoTextBottom = "<div style='margin-top: 20px;'>More configuration options can be added here...</div>";

  WiFiManager wm;
  wm.setSaveConfigCallback(saveConfigCallback);
  // Callback que se dispara si falla la conexión previa y entra en modo AP
  wm.setAPCallback(configModeCallback);

  // Añadir el texto sobre los parámetros
  WiFiManagerParameter config_info_top(configInfoText);
  wm.addParameter(&config_info_top);

  // --- parámetros de configuración adicionales ---

  // Campo de texto (cadena)
  WiFiManagerParameter custom_text_box("key_text", "Enter your Name", testString, 50); // 50 == max length

  // Text box (Number)
  sprintf(testNumberStr, "%llu", testNumber); // Convertir testNumber a cadena
  WiFiManagerParameter custom_text_box_num("key_num", "Enter Emergency Contact's number", testNumberStr, 12); // 12 == max length

  // Campo de texto (API Key)
  char convertedValueApi[7];
  sprintf(convertedValueApi, "%d", apikey); // Convertir a cadena para mostrar valor por defecto
  WiFiManagerParameter custom_text_box_api("key_api", "Enter Emergency contact's API key", convertedValueApi, 7); // 7 == max length


  // Añade aquí todos tus parámetros
  wm.addParameter(&custom_text_box);
  wm.addParameter(&custom_text_box_num);
  wm.addParameter(&custom_text_box_api);

  // Añadir el texto bajo los parámetros
  WiFiManagerParameter config_info_bottom(configInfoTextBottom);
  wm.addParameter(&config_info_bottom);
  Serial.println("hello");

  digitalWrite(PIN_LED, LOW);
  if (forceConfig)
  {
    if (!wm.startConfigPortal("Fall_detector", "clock123"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // Reiniciar y reintentar; o entrar en deep sleep
      ESP.restart();
      delay(5000);
    }
  }
  else
  {
    if (!wm.autoConnect("Fall_detector", "clock123"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // Si aún no conecta, reiniciar y probar de nuevo
      ESP.restart();
      delay(5000);
    }
  }

  // Si llegamos aquí, estamos conectados a la WiFi
  WiFi.config(staticIP, gateway, subnet, dns);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  digitalWrite(PIN_LED, HIGH);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Procesar los valores de configuración del usuario

  // Copiar el valor de cadena
  strncpy(testString, custom_text_box.getValue(), sizeof(testString));
  Serial.print("testString: ");
  Serial.println(testString);

  // Convertir el valor numérico
  testNumber = strtoull(custom_text_box_num.getValue(), nullptr, 10);
  sprintf(testNumberStr, "%llu", strtoull(custom_text_box_num.getValue(), nullptr, 10));
  Serial.print("testNumber: ");
  Serial.println(testNumberStr);

  apikey = atoi(custom_text_box_api.getValue());
  Serial.print("apikey: ");
  Serial.println(apikey);

  // Guardar parámetros personalizados en FS
  if (shouldSaveConfig)
  {
    saveConfigFile();
  }
}

// ... Resto del código ...

void loop()
{
  drd->loop();
  printLocalTime();
  HTTPClient http;
  char encodedDateTimeStr[50];
  String encodedDateTime = urlEncode(dateTimeStr);

  char testNumberStr[20]; // Allocate a char array to hold the converted number as a string
  sprintf(testNumberStr, "%llu", testNumber); // Convert the testNumber to a string

  String serverPath = serverName + "phone=" + String(testNumberStr) + "&apikey=" + String(apikey) + "&text=A+Fall+has+been+detected+for+user:+" + testString + "+on+" + encodedDateTime;

  http.begin(serverPath.c_str());
  static unsigned long lastTime = 0;

  // Leer datos del acelerómetro
  if (millis() - lastTime >= sampleInterval)
  {
    lastTime = millis();

    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    // Convertir lecturas del acelerómetro de m/s^2 a mg (1 g = 9.8 m/s^2)
    float acceleration_mg_x = ax / 9.8;
    float acceleration_mg_y = ay / 9.8;
    float acceleration_mg_z = az / 9.8;

    // Calcular el jerk como derivada de aceleración respecto al tiempo
    float jerkX = (acceleration_mg_x - prevAccX) / (sampleInterval / 1000.0);
    float jerkY = (acceleration_mg_y - prevAccY) / (sampleInterval / 1000.0);
    float jerkZ = (acceleration_mg_z - prevAccZ) / (sampleInterval / 1000.0);

    // Actualizar las aceleraciones previas para la siguiente iteración
    prevAccX = acceleration_mg_x;
    prevAccY = acceleration_mg_y;
    prevAccZ = acceleration_mg_z;

    // Calcular la magnitud del jerk
    float jerkMagnitude = sqrt(jerkX * jerkX + jerkY * jerkY + jerkZ * jerkZ);
    //Serial.println(jerkMagnitude);
    // Comprobar si hay caída
    if (jerkMagnitude > fallThreshold)
    {
      // Fall detected
      Serial.println("Fall detected!");
      ledcWriteTone(BUZZER_CHANNEL, 5000); // Reproducir tono de 5 kHz en el buzzer

      // Reiniciar la bandera de interrupción por botón
      buttonInterrupted = false;

      delayStartTime = millis();
      while (delayStartTime > 0 && millis() - delayStartTime < BEEP_DURATION)
      {
        if (button1.pressed)
        {
          button1.pressed = false;
          ledcWrite(BUZZER_CHANNEL, 0); // Stop the tone
          Serial.println("Buzzer stopped by button press.");
          break;
        }
      }

      if (!buttonInterrupted) {
      // Sin pulsación de botón: enviar la petición HTTP
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        Serial.println("Alert message sent successfully!");
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        // Actualizar estado y hora de la última detección de caída
        lastFallDetection = true;
        lastFallTime = millis();
        } else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        // Liberar recursos
        http.end();
        //ledcWrite(BUZZER_CHANNEL, 0);
      }
      delayStartTime = 0;
    }
    if (button1.pressed)
    {
      if (delayStartTime > 0)
      {
        delayStartTime = 0;
      }
      Serial.printf("Stop Button was pressed %u times\n", button1.numberKeyPresses);
      button1.pressed = false;
      ledcWrite(BUZZER_CHANNEL, 0); // Detener el tono
    }
  }
  // Comprobar doble pulsación
  if (button1.numberKeyPresses >= 2 && (millis() - button_time) <= DOUBLE_PRESS_THRESHOLD) {
    Serial.println("Double press detected!");
    if (lastFallDetection) {
      // Enviar petición HTTP con el mensaje de seguridad
      String safeMessage = "Last+fall+detection+was+false%2C+user:+%22" + String(testString) + "%22+is+safe";
      String serverPath = serverName + "phone=" + String(testNumberStr) + "&apikey=" + String(apikey) + "&text=" + safeMessage;
      http.begin(serverPath.c_str());
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        Serial.println("Safe message sent successfully!");
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      lastFallDetection = false;
    } else {
      Serial.println("No previous fall detection to send safe message.");
    }
    // Reiniciar el contador de pulsaciones del botón
    button1.numberKeyPresses = 0;
  }
}
