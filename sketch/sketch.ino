#include <esp_crt_bundle.h>
#include <ssl_client.h>
#include <WiFiClientSecure.h>

#include "PubSubClient.h"

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// MQTT details
const char *broker = "5dda2b51fb16459eb2e2de9ea499b546.s1.eu.hivemq.cloud"; // Public IP address or domain name
const char *mqttUsername = "admin";                                         // MQTT username
const char *mqttPassword = "Admin123";                                      // MQTT password
const int mqttPort = 8883;

const char *topicEngine = "esp/engine";
const char *topicGPS = "esp/gps";

#define WIFI_AP "kamar tengah"
#define WIFI_PASS "kamar123"

uint32_t lastReconnectAttempt = 0;

WiFiClientSecure espClient;

PubSubClient mqtt(espClient);

#define ENGINE_1 15

struct location
{
    double lat;
    double lng;
};

long lastMsg = 0;

location currentLocation;

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

void connectToWiFi()
{
    Serial.println("Connecting to WiFi...");
    int attempts = 0;

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_AP, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        // spinner();
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\nFailed to connect to WiFi");
        delay(3000);
    }
    else
    {
        Serial.println("\nConnected to WiFi");
        delay(3000);
    }
}

void mqttCallback(char *topic, byte *message, unsigned int len)
{
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < len; i++)
    {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();

    // Feel free to add more if statements to control more GPIOs with MQTT

    // If a message is received on the topic esp/output1, you check if the message is either "true" or "false".
    // Changes the output state according to the message
    Serial.println("Changing output to ");
    Serial.println(messageTemp);
    if (messageTemp == "true")
    {
        Serial.println("true");
        // digitalWrite(ENGINE_1, HIGH);
    }
    else if (messageTemp == "false")
    {
        Serial.println("false");
        // digitalWrite(ENGINE_1, LOW);
    }
}

boolean mqttConnect()
{
    SerialMon.print("Connecting to ");
    SerialMon.print(broker);

    // Connect to MQTT Broker without username and password
    // boolean status = mqtt.connect("GsmClientN");

    // Or, if you want to authenticate MQTT:
    boolean status = mqtt.connect("GsmClientN", mqttUsername, mqttPassword);
    if (!status)
    {
        SerialMon.println(" fail");
        ESP.restart();
        return false;
    }
    SerialMon.println(" success");
    mqtt.subscribe(topicEngine);

    return status;
}

void setup()
{
    Serial.begin(115200);
    Serial.println(F("MQTT test!"));

    pinMode(ENGINE_1, OUTPUT);

    SerialMon.println("Wait...");

    connectToWiFi();

    // MQTT Broker setup
    espClient.setCACert(root_ca);
    mqtt.setServer(broker, mqttPort);
    mqtt.setCallback(mqttCallback);
}

void loop()
{
    if (!mqtt.connected())
    {
        mqttConnect();
        // return;
    }

    mqtt.loop();
}