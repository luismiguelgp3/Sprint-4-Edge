# Sprint 4 - Passa a Bola - Poc

## Engenharia de Software - 1ESPZ - FIAP

> Prova de conceito completa: **ESP32 (Wokwi) → MQTT Broker → IoT Agent (Ultralight) → Orion Context Broker** com consulta e testes via **Postman/cURL**, código reproduzível e estrutura organizada.

---

## 👥 Integrantes

* **Camila de Mendonça da Silva** - RM: 565491
* **Davi Alves dos Santos** - RM: 566279
* **Rafael Joda** - RM: 561939
* **Luis Miguel** - RM: 561232
* **Diego Zandonadi** - RM: 561488

---

## 🗺️ Visão Geral da Arquitetura

**Botões → ESP32 (Wokwi) → MQTT → IoT Agent (Ultralight/MQTT) → Orion Context Broker → Postman**

* O ESP32 publica leituras por **MQTT** nos tópicos `…/attrs/*`.
* O **IoT Agent (Ultralight)** consome essas publicações e **atualiza atributos** da entidade no **Orion** (NGSI v2).
* **Postman** é usado para **consultar/validar** a entidade no Orion (GET /v2/entities).
* (Opcional) **Comandos** podem ser enviados do Orion → IoT Agent → ESP32 (o firmware já ouve `/cmd` e liga/desliga um pino).

---

## 📁 Estrutura do Repositório

```
Sprint-4-Edge/
├─ README.md                              # este documento
├─ docs/
│  ├─ prints-wokwi/
├─ firmware/
│  └─ main.cpp                            # código final do ESP32 (Wokwi) - MQTT/Ultralight    
└─                   
```

---

## 🧩 Mapeamento (Atributos → Orion via IoT Agent)

| Botões       | Variável (código) | Tópico MQTT                      | Atributo no Orion | 
| ------------ | ----------------- | -------------------------------- | ----------------- | 
| Escanteio    | `escanteio`       | `/TEF/bandeira003/attrs/esc`     | `escanteio`       | 
| Lateral      | `lateral`         | `/TEF/bandeira003/attrs/lat`     | `lateral`         | 
| Impedimento  | `impedimento`     | `/TEF/bandeira003/attrs/imp`     | `impedimento`     | 
| Tiro de Meta | `tirodemeta`      | `/TEF/bandeira003/attrs/tim`     | `tirodemeta`      | 

> **Comandos** 
> O **IoT Agent** deve estar configurado para mapear `esc,imp,lat,tim` → atributos da entidade `bandeira003` (ou `BANDEIRA001`, conforme cadastro).

---

## 🔧 Pré-requisitos

* **Broker MQTT** acessível (ex.: Mosquitto).
* **FIWARE IoT Agent Ultralight (MQTT)** configurado e apontando para o **Orion**.
* **Orion Context Broker** ativo (NGSI v2).
* **Postman** (ou cURL) para validações.
* **Wokwi** (ESP32) – rede padrão: `Wokwi-GUEST`.

> **Endpoints de exemplo (ajuste para o seu ambiente)**
>
> * Broker MQTT: `172.24.240.1:1883`
> * IoT Agent: `http://<iot-agent-host>:4041`
> * Orion: `http://<orion-host>:1026`

---

## 💻 Código Final (ESP32 → MQTT → Orion)

> Cole em `firmware/main.cpp`. Ajuste **BROKER**, **TOPICS** e **DEVICE** se necessário.

```cpp
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
const int botaoTirodeMeta     = 15;

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
```

> **Destaques do firmware:**
>
> * Publica `esc,imp,lat,tim` em `/TEF/bandeira003/attrs/*`.

---

## 🔌 Pinagem (Wokwi)

[**🔗 Link do projeto do Wokwi**](https://wokwi.com/projects/442197804454616065)


* **Botão-Escanteio:** btn1:1.r→GND, btn1:2.r→GPIO12
* **Botão-Lateral:** btn2:1.r→GND, btn2:2.r→GPIO13
* **Botão-Impedimento:** btn3:1.r→GND, btn3:2.r→GPIO14
* **Botão-Tiro-de-Meta:** btn4:1.r→GND, btn4:2.r→GPIO15

---

# 📸 Imagens

---

## Hardware de Simulação Montado (Wokwi)

<img width="445" height="478" alt="image" src="https://github.com/user-attachments/assets/06ffb5bd-5dd6-498a-82e3-71b9d07e73d0" />

---

## HTTPS 200 confirmando êxito (Wokwi)
<img width="216" height="51" alt="https200" src="https://github.com/user-attachments/assets/0be2d245-c40f-4ca1-91f5-ab651859acd9" />

---

# 📸 Evidências de funcionamento do Postman
---

<img width="1919" height="1079" alt="imagem_codigo" src="https://github.com/user-attachments/assets/3cd9d0e5-c03b-4ade-969d-d5cbb4841d5d" />

---

<img width="1919" height="1079" alt="imagem_codigo2" src="https://github.com/user-attachments/assets/469f77a7-8fe6-43b1-bf86-2c0fef906d7e" />

---

<img width="1918" height="1079" alt="imagem_codigo3" src="https://github.com/user-attachments/assets/031cf4e3-8854-4165-86b3-5bf293be0234" />

---

<img width="1919" height="1079" alt="imagem_codigo44" src="https://github.com/user-attachments/assets/f1027a36-f7b1-480a-995e-582237ed3d4d" />

---

<img width="1919" height="1079" alt="print1" src="https://github.com/user-attachments/assets/7970eec1-b7e3-4a88-b902-aeccae085b00" />

---

<img width="1919" height="1079" alt="print2" src="https://github.com/user-attachments/assets/5a74ee90-22c4-4d4d-8ef8-a843e954657c" />

---


## 🧪 Validação com Postman (Orion)

> **Pré-condição:** O **IoT Agent** já registrou o **device** e mapeamentos (service, servicepath, deviceId, atributos `esc,imp,lat,tim`).
> Assim que o ESP32 publicar em MQTT, o IoT Agent **atualiza** a entidade no Orion.

**1) Consultar entidade (NGSI v2)**

```
GET http://<orion-host>:1026/v2/entities/<ENTITY_ID>
```

**Resposta esperada (exemplo):**

```json
{
  "id": "bandeira003",
  "type": "WineCellar",
  "escanteio":    { "value": 0, "type": "Number" },
  "lateral":      { "value": 0, "type": "Number" },
  "impedimento":  { "value": 0, "type": "Number" },
  "tirodemeta":   { "value": 0, "type": "Number" },
}
```

**2) (Opcional) Enviar comando para o dispositivo**

> Forma recomendada quando o device está **cadastrado no IoT Agent** como *command*:

```
POST http://<iot-agent-host>:4041/iot/devices/<DEVICE_ID>/commands
Content-Type: application/json

{ "on": "" }
```

## 🔁 Replicabilidade (passo a passo)

1. **Confirmar ambiente**: Broker MQTT, IoT Agent Ultralight (MQTT) e Orion ativos.
2. **Cadastrar device** no IoT Agent (mapeando `esc,imp,lat,tim` → Orion).
3. **Abrir Wokwi**, carregar `firmware/main.cpp`, conferir **IP do broker** e **TOPICS**.
4. **Run** no Wokwi e observar **Serial** (conexão Wi-Fi, MQTT e publicações).
5. No **Postman**, usar `GET /v2/entities/<ENTITY_ID>` no Orion e validar atributos.

---

## 🧪 Resultados da PoC

* **Coleta confiável** de **escanteio, lateral, impedimento e tiro de meta**.
* **Publicação MQTT** e **ingestão via IoT Agent** com atualização no **Orion**.
* **Validação Postman** via `GET /v2/entities/<id>` (NGSI v2).
* **Comandos** do backend para o dispositivo (Orion/IoT Agent → MQTT → ESP32).

---
