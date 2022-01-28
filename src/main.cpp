/*
 * Created By : Partha Singha Roy & Sunirban Ranjit
 *
 * Date : 29.01.2022
 *
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

/* --- Macros --- */
#define USE_SERIAL Serial // USB port
#define R D1
#define G D5
#define B D7
#define BAUD_RATE 115200
#define SSID "vivo 1816"
#define PASSWORD "12345PSR"
#define ADDRESS "test-code-0001.herokuapp.com"
#define PORT 80
#define URL "/socket.io/?EIO=4"

/* --- GLobals --- */
SocketIOclient socketIO;

/* --- Function prototypes --- */
void socketIOEventHandler(socketIOmessageType_t type, uint8_t *payload, size_t length);
void payloadHandler(uint8_t *payload);
void rgb(int red, int green, int blue);
int hexToDec(String hexVal);
void set_color(String hex);

/* --- Driver Setup --- */
void setup()
{
    USE_SERIAL.begin(BAUD_RATE);
    pinMode(R, OUTPUT);
    pinMode(G, OUTPUT);
    pinMode(B, OUTPUT);

    analogWriteFreq(500);
    // for ease of debugging
    USE_SERIAL.setDebugOutput(true);

    for (uint8_t t = 4; t > 0; t--)
    {
        USE_SERIAL.printf("  [SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    // disable AP
    if (WiFi.getMode() & WIFI_AP)
    {
        WiFi.softAPdisconnect(true);
    }

    // WiFi begins
    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        USE_SERIAL.print(".");
        delay(500);
    }

    // local ip address
    String ip = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

    // server address, port and URL
    socketIO.begin(ADDRESS, PORT, URL);

    // event handler
    socketIO.onEvent(socketIOEventHandler);
}
/* -------- Driver Loop --------- */
void loop()
{
    socketIO.loop();
}

/* --------- Function definitions --------- */
// event handler Function ---->
void socketIOEventHandler(socketIOmessageType_t type, uint8_t *payload, size_t length){
    switch (type)
    {
    case sIOtype_DISCONNECT:
        USE_SERIAL.printf("[IOc] Disconnected!\n");
        break;
    case sIOtype_CONNECT:
        USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);

        // join default namespace (no auto join in Socket.IO V3)
        socketIO.send(sIOtype_CONNECT, "/");
        break;
    case sIOtype_EVENT:
        // handel all the event coming from the NODE server
        USE_SERIAL.printf("[IOc] get event: %s\n", payload);
        // serve the payload to payload handler function
        payloadHandler(payload);
        break;
    case sIOtype_ACK:
        USE_SERIAL.printf("[IOc] get ack: %u\n", length);
        hexdump(payload, length);
        break;
    case sIOtype_ERROR:
        USE_SERIAL.printf("[IOc] get error: %u\n", length);
        hexdump(payload, length);
        break;
    case sIOtype_BINARY_EVENT:
        USE_SERIAL.printf("[IOc] get binary: %u\n", length);
        hexdump(payload, length);
        break;
    case sIOtype_BINARY_ACK:
        USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
        hexdump(payload, length);
        break;
    }
}

// payload Handler Function--->
void payloadHandler(uint8_t *payload){
    StaticJsonDocument<80> doc;
    deserializeJson(doc, (const char *)payload);
    JsonArray array = doc.as<JsonArray>();

    String topic = array[0].as<const char *>();
    if (topic == "RGB-Data")
    {
        String hex = array[1].as<const char *>();
        set_color(hex);
    }
    else
        return;
    delay(10);
}

void set_color(String hex){
    int r, g, b;
    if (hex.length() != 7){
        USE_SERIAL.println("Invalid color code");
        return;
    }
    r = hexToDec(hex.substring(1, 3));
    g = hexToDec(hex.substring(3, 5));
    b = hexToDec(hex.substring(5));
    USE_SERIAL.println("");
    rgb(r, g, b);
}

void rgb(int red, int green, int blue)
{
    analogWrite(R, red);
    analogWrite(G, green);
    analogWrite(B, blue);
}

//hex to decimal converter function
int hexToDec(String hexVal)
{
    int len = hexVal.length();
    int base = 1, i = 0;
    int dec_val = 0;

    for (i = 0; hexVal[i] != '\0'; i++){
        if (hexVal[i] >= 'a' && hexVal[i] <= 'z')
            hexVal[i] = hexVal[i] - 32;
    }
    for (int i = len - 1; i >= 0; i--){
        if (hexVal[i] >= '0' && hexVal[i] <= '9')
        {
            dec_val += (int(hexVal[i]) - 48) * base;
            // incrementing base by power
            base = base * 16;
        }
        else if (hexVal[i] >= 'A' && hexVal[i] <= 'F'){
            dec_val += (int(hexVal[i]) - 55) * base;
            base = base * 16;
        }
    }
    return dec_val;
}