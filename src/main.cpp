#include <main.h>

CRGB leds[numLeds];
CRGB leds_temp[numLeds / 2];
CRGB audioBuffer[numLeds];

const uint8_t smoothing = 190;
CMSGEQ7<smoothing, pinReset, pinStrobe, analogPin> MSGEQ7;

const char *ssid = "MySpectrumWiFic0-2G";
const char *password = "widebunny005";
const uint8_t port = 80;

const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(10) + 210;
DynamicJsonDocument doc(capacity);

CRGBPalette16 currentPalette;
uint8_t gHue = 0;

uint8_t modeIndex;
bool hasSetOff = false;

double brightness;
bool finished = false;
uint8_t on;

void setup()
{
  Serial.begin(115200);
  delay(300);
  FastLED.addLeds<WS2812B, pinData, GRB>(leds, numLeds).setCorrection(TypicalLEDStrip);
  FastLED.setDither(false);
  pinMode(analogPin, INPUT);
  pinMode(pinStrobe, OUTPUT);
  pinMode(pinReset, OUTPUT);

  digitalWrite(pinReset, LOW);
  digitalWrite(pinStrobe, HIGH);

  fill_solid(leds, numLeds, CHSV(0, 0, 0));
  FastLED.show();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(2000);
    Serial.println("Connecting..");
  }

  Serial.print("Connected to WiFi. IP:");
  Serial.println(WiFi.localIP());
  Serial.print("Port:");
  Serial.println(port);

  AsyncServer *server = new AsyncServer(port);
  server->onClient(&handleNewClient, server);
  server->begin();
}

void loop()
{
  if (!on)
  {
    fill_solid(leds, numLeds, CRGB::Black);
    FastLED.show();
    return;
  }

  EVERY_N_MILLISECONDS(40)
  {
    gHue++;
  }

  hasSetOff = false;
  modes[modeIndex].mode();
  FastLED.setBrightness(brightness);
  FastLED.show();
}

static void handleNewClient(void *arg, AsyncClient *client)
{
  // register events
  client->onData(&handleData, NULL);
  client->onError(&handleError, NULL);
  client->onDisconnect(&handleDisconnect, NULL);
  client->onTimeout(&handleTimeOut, NULL);
}

static void handleError(void *arg, AsyncClient *client, int8_t error)
{
  //Free's Heap
  delete client;
}

static void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
  DeserializationError err = deserializeJson(doc, (uint8_t *)data, len);
  if (err)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(err.c_str());
    return;
  }

  if (doc.isNull())
  {
    return;
  }

  setConfig(doc["mode"].as<String>());
}

static void handleDisconnect(void *arg, AsyncClient *client)
{
  //Free's Heap
  //Serial.println("Disconnecting from client...");
  delete client;
}

static void handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
  //Free's Heap
  //Serial.println("Client timeout...");
  delete client;
}

//CRGBPalette16 pal = Rainbow_gp;

void waves()
{
  if (!(doc["colors"].isNull()))
  {
    currentPalette = createNewPalette(doc["colors"].as<JsonArray>());
  }

  colorwaves(leds, numLeds, currentPalette);
}

void solid()
{
  if (!(doc["color"].isNull()))
  {
    currentPalette = CRGBPalette16(doc["color"].as<long>());
  }

  fill_palette(leds, numLeds, 0, 0, currentPalette, brightness, NOBLEND);
}

long interval = 10;   
long previousMillis = 0;     
 
void music()
{

  MSGEQ7.begin();
  long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    bool newReading = MSGEQ7.read();
    if (newReading)
    {
      previousMillis = currentMillis;   
      musicVisualizer();
      otherHalf();
      //FastLED.delay(6);

    }
  }
}

void ambilight()
{
}

void musicVisualizer()
{
  //After the device does not receive data from arduinoJSON, it thros some random value for the key.
  if (!doc["low"].isNull() || !doc["med"].isNull() || !doc["high"].isNull())
  {
    currentPalette = CRGBPalette16(doc["low"].as<long>(), doc["med"].as<long>(), doc["high"].as<long>());
  }

  //Shift LED values forward https://github.com/PhilKes/RGBStripe_Control_WS2812/blob/master/RGBStripe_AudioVisualizer_only/RGBStripe_AudioVisualizer_only.ino
  for (int k = (numLeds / 2) - 1; k > 0; k--)
  {
    audioBuffer[k] = audioBuffer[k - 1];
  }

  audioBuffer[0] = ColorFromPalette(currentPalette, 0, mapNoise(MSGEQ7.get(MSGEQ7_LOW), 60, 255, 0, 255), NOBLEND);
  audioBuffer[0] |= ColorFromPalette(currentPalette, 128, mapNoise(MSGEQ7.get(MSGEQ7_MID), 90, 255, 0, 255), NOBLEND);
  audioBuffer[0] |= ColorFromPalette(currentPalette, 255, mapNoise(MSGEQ7.get(MSGEQ7_HIGH), 95, 255, 0, 255), NOBLEND);

  //audioBuffer[0] = ColorFromPalette(currentPalette, 0, map(mapNoise(MSGEQ7.get(MSGEQ7_LOW), 260, 1024, 0, 1024), 0, 1024, 0, 255), NOBLEND);
  //audioBuffer[0] |= ColorFromPalette(currentPalette, 128, map(mapNoise(MSGEQ7.get(MSGEQ7_MID), 365, 1024, 0, 1024), 0 ,1024, 0 , 255), NOBLEND);
  //audioBuffer[0] |= ColorFromPalette(currentPalette, 255, map(mapNoise(MSGEQ7.get(MSGEQ7_HIGH), 285, 1024, 0, 1024), 0, 1024, 0, 255), NOBLEND);
  
  //Send new LED values to WS2812
  for (int i = 0; i < numLeds / 2; i++)
  {
    leds_temp[i] = audioBuffer[i];
  }
}

void otherHalf()
{
  bool gReverseDirection = false;
  if (!gReverseDirection)
  { //false is center outward
    for (uint8_t i = 0; i < numLeds / 2; i++)
    {
      leds[(numLeds / 2) - 1 - i] = leds_temp[i];
      leds[(numLeds / 2) + i] = leds_temp[i];
    }
  }
  else
  {
    //true is from ends inward
    for (uint8_t i = 0; i < numLeds / 2; i++)
    {
      leds[i] = leds_temp[i];
      leds[(numLeds - 1) - i] = leds_temp[i];
    }
  }
}

void breathing()
{

  if (!(doc["colors"].isNull()))
  {
    currentPalette = createNewPalette(doc["colors"].as<JsonArray>());
  }

  for (int i = 0; i < numLeds; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, gHue, brightness, LINEARBLEND);
    //colorIndex += 3;
  }
}

void setConfig(String mode)
{
  on = doc["on"].as<bool>();
  if (!on)
  {
    MSGEQ7.end();
    return;
  }

  brightness = (doc["brightness"].as<double>() / 100) * 255;

  if (mode.equals("DeviceWaves"))
  {
    MSGEQ7.end();
    modeIndex = 0;
  }
  else if (mode.equals("DeviceSolid"))
  {
    MSGEQ7.end();
    modeIndex = 1;
  }
  else if (mode.equals("DeviceMusic"))
  {
    modeIndex = 2;
  }
  else if (mode.equals("DeviceAmbilight"))
  {
    MSGEQ7.end();
    modeIndex = 3;
  }
  else if (mode.equals("DeviceBreathing"))
  {
    MSGEQ7.end();
    modeIndex = 4;
  }
}