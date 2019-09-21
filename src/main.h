#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <colors.h>
#include <modes.h>
#include <MSGEQ7.h>

#define pinData D5
#define numLeds 94
#define analogPin A0
#define pinReset D2
#define pinStrobe D3

void musicVisualizer();
void otherHalf();
void waves();
void music();
void solid();
void ambilight();
void breathing();

void setConfig(String mode);
static void handleNewClient(void *arg, AsyncClient *client);
static void handleError(void *arg, AsyncClient *client, int8_t error);
static void handleData(void *arg, AsyncClient *client, void *data, size_t len);
static void handleDisconnect(void *arg, AsyncClient *client);
static void handleTimeOut(void *arg, AsyncClient *client, uint32_t time);

typedef void (*Modes)();
typedef Modes PatternList[];
typedef struct
{
  Modes mode;
  String name;
} ModesAndName;
typedef ModesAndName ModesAndNameList[];

ModesAndNameList modes = {
    {waves, "DeviceWaves"},
    {solid, "DeviceSolid"},
    {music, "DeviceMusic"},
    {ambilight, "DeviceAmbilight"},
    {breathing, "DeviceBreathing"}
};
