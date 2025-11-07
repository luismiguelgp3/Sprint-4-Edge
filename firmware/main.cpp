#include <WiFi.h>
#include <PubSubClient.h>

const char* default_SSID = "Wokwi-GUEST";   // Nome da rede Wi-Fi
const char* default_PASSWORD = "";           // Senha da rede Wi-Fi

const char* default_BROKER_MQTT = "172.24.240.1"; // IP do Broker MQTT
const int default_BROKER_PORT = 1883;              // Porta do Broker MQTT
const char* default_TOPICO_PUBLISH_ESCANTEIO = "/TEF/bandeira002/attrs/esc";
const char* default_TOPICO_PUBLISH_LATERAL   = "/TEF/bandeira002/attrs/lat";
const char* default_TOPICO_PUBLISH_IMPED     = "/TEF/bandeira002/attrs/imp";
const char* default_TOPICO_PUBLISH_TIRODEMETA     = "/TEF/bandeira002/attrs/tim";
const char* default_ID_MQTT = "fiware_001";

const int botaoEscanteio = 12;
const int botaoLateral   = 13;
const int botaoImped     = 14;
const int botaoTirodeMeta = 15;

// Variáveis de estado (debounce)
int lastStateEscanteio = HIGH;
int lastStateLateral   = HIGH;
int lastStateImped     = HIGH;
int lastStateTirodeMeta     = HIGH;

WiFiClient espClient;
PubSubClient MQTT(espClient);

void initSerial() {
  Serial.begin(115200);
}

void initWiFi() {
  delay(10);
  Serial.println("------ Conexão Wi-Fi ------");
  WiFi.begin(default_SSID, default_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado com sucesso!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  MQTT.setServer(default_BROKER_MQTT, default_BROKER_PORT);
}

void setup() {
  initSerial();
  initWiFi();
  initMQTT();

  pinMode(botaoEscanteio, INPUT_PULLUP);
  pinMode(botaoLateral, INPUT_PULLUP);
  pinMode(botaoImped, INPUT_PULLUP);
  pinMode(botaoTirodeMeta, INPUT_PULLUP);

  Serial.println("Sistema pronto. Aguardando botões...");
}

void loop() {
  VerificaConexoesWiFIEMQTT();
  verificarBotoes();
  MQTT.loop();
}

void verificarBotoes() {
  int currentEscanteio = digitalRead(botaoEscanteio);
  int currentLateral   = digitalRead(botaoLateral);
  int currentImped     = digitalRead(botaoImped);
  int currentTirodeMeta     = digitalRead(botaoTirodeMeta);

  if (currentEscanteio == LOW && lastStateEscanteio == HIGH) {
    enviarEvento(default_TOPICO_PUBLISH_ESCANTEIO, "1");
    Serial.println("Escanteio enviado via MQTT");
    delay(50);
  }
  lastStateEscanteio = currentEscanteio;

  if (currentLateral == LOW && lastStateLateral == HIGH) {
    enviarEvento(default_TOPICO_PUBLISH_LATERAL, "1");
    Serial.println("Lateral enviada via MQTT");
    delay(50);
  }
  lastStateLateral = currentLateral;

  if (currentImped == LOW && lastStateImped == HIGH) {
    enviarEvento(default_TOPICO_PUBLISH_IMPED, "1");
    Serial.println("Impedimento enviado via MQTT");
    delay(50);
  }
  lastStateImped = currentImped;

  if (currentTirodeMeta == LOW && lastStateTirodeMeta == HIGH) {
    enviarEvento(default_TOPICO_PUBLISH_TIRODEMETA, "1");
    Serial.println("Tiro de Meta enviado via MQTT");
    delay(50);
  }
  lastStateTirodeMeta = currentTirodeMeta;
}

void enviarEvento(const char* topico, const char* mensagem) {
  if (MQTT.connected()) {
    MQTT.publish(topico, mensagem);
  } else {
    Serial.println("Não conectado ao broker MQTT. Tentando reconectar...");
    reconnectMQTT();
  }
}

void VerificaConexoesWiFIEMQTT() {
  if (!MQTT.connected()) {
    reconnectMQTT();
  }
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(default_BROKER_MQTT);
    if (MQTT.connect(default_ID_MQTT)) {
      Serial.println("Conectado ao broker MQTT com sucesso!");
    } else {
      Serial.println("Falha na conexão. Tentando novamente em 2 segundos...");
      delay(2000);
    }
  }
}
