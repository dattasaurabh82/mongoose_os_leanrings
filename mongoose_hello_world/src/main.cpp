#include <Arduino.h>
#include "WiFi.h"
#include "mongoose.h"
#include "credentials.h"


#define SERVER_PORT "80"      // Server port
#define LED_PIN LED_BUILTIN   // LED pin

// #define WIFI_SSID "YOUR WIFI SSID"      // Change this in include/credentials.h (check include/credentials.h.example)
// #define WIFI_PASS "YOUR WIFI PASSWORD"  // Change this in include/credentials.h (check include/credentials.h.example)

// Mongoose Object
struct mg_mgr mgr;

// Mongoose Event Handler
void http_ev_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {

    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    if (mg_match(hm->uri, mg_str("/api/led/on"), NULL)) {
      digitalWrite(LED_PIN, HIGH);
      mg_http_reply(c, 200, "", "{%m: %d}\n", MG_ESC("led"), digitalRead(LED_PIN));
    } else if (mg_match(hm->uri, mg_str("/api/led/off"), NULL)) {
      digitalWrite(LED_PIN, LOW);
      mg_http_reply(c, 200, "", "{%m: %d}\n", MG_ESC("led"), digitalRead(LED_PIN));
    } else {
      mg_http_reply(c, 200, "", "{%m: %u}\n", MG_ESC("free RAM"), xPortGetFreeHeapSize());
    }
  }
}

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    delay(50);
  }

  pinMode(LED_PIN, OUTPUT);

  mg_mgr_init(&mgr);        // Init the mongoose instance
  mg_log_set(MG_LL_DEBUG);  // Set log level from within mongoose as we will not sure serial print from now on ...
  mg_log_set_fn([](char ch, void *) {
    Serial.print(ch);
  },
                NULL);

  // Connect to Wifi access point
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  mg_log("\nConnected to the WiFi network\n");
  mg_log("IP Address: %s", WiFi.localIP().toString().c_str());

  // Start the HTTP server
  mg_http_listen(&mgr, "http://0.0.0.0:8080", http_ev_handler, NULL);
}

void loop() {
  mg_mgr_poll(&mgr, 0);
}

extern "C" int lwip_hook_ip6_input(struct pbuf *p, struct netif *inp) __attribute__((weak));
extern "C" int lwip_hook_ip6_input(struct pbuf *p, struct netif *inp) {
  if (ip6_addr_isany_val(inp->ip6_addr[0].u_addr.ip6)) {
    pbuf_free(p);
    return 1;
  }
  return 0;
}