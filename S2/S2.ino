#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "env.h"

WiFiClientSecure conexaoSegura;
PubSubClient brokerMqtt(conexaoSegura);

const byte trigger1 = 22;
const byte echo1 = 23;
const byte trigger2 = 12;
const byte echo2 = 13;

const byte ledR = 14;
const byte ledG = 26;
const byte ledB = 25;

int medida1 = 0;
int medida2 = 0;
bool ultimoEstado1 = false;
bool ultimoEstado2 = false;

int calcularDistancia(byte pino_echo, byte pino_trigger) {
  digitalWrite(pino_trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pino_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pino_trigger, LOW);
  
  unsigned long tempo = pulseIn(pino_echo, HIGH, 30000);
  if (tempo == 0) return -1;
  return (tempo * 0.0343) / 2;
}

void alterarLed(byte vermelho, byte verde, byte azul) {
  ledcWrite(ledR, vermelho);
  ledcWrite(ledG, verde);
  ledcWrite(ledB, azul);
}

void conectarWifi() {
  WiFi.begin(SSID, PASS);
  Serial.print("Estabelecendo conexao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    alterarLed(0, 0, 255);
    delay(300);
  }
  Serial.println("\nConexao WiFi estabelecida!");
  alterarLed(0, 255, 0);
}

void estabelecerConexaoBroker() {
  Serial.print("Conectando ao servidor MQTT");
  String identificador = "S2-" + String(random(0xffff), HEX);
  while (!brokerMqtt.connected()) {
    if (brokerMqtt.connect(identificador.c_str(), BROKER_USER_NAME, BROKER_USER_PASS)) {
      Serial.println("\nServidor MQTT conectado!");
      alterarLed(0, 255, 0);
    } else {
      Serial.print(".");
      delay(500);
    }
  }
}

void reconectarWifi() {
  Serial.println("Reestabelecendo conexao WiFi...");
  WiFi.disconnect();
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi reconectado!");
}

void setup() {
  Serial.begin(115200);
  
  ledcAttach(ledR, 5000, 8);
  ledcAttach(ledG, 5000, 8);
  ledcAttach(ledB, 5000, 8);
  alterarLed(255, 0, 0);
  
  conexaoSegura.setInsecure();
  
  pinMode(trigger1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trigger2, OUTPUT);
  pinMode(echo2, INPUT);
  
  conectarWifi();
  brokerMqtt.setServer(BROKER_URL, BROKER_PORT);
  estabelecerConexaoBroker();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    reconectarWifi();
  }
  
  if (!brokerMqtt.connected()) {
    estabelecerConexaoBroker();
  }
  
  brokerMqtt.loop();
  
  medida1 = calcularDistancia(echo1, trigger1);
  medida2 = calcularDistancia(echo2, trigger2);
  
  bool estadoAtual1 = (medida1 > 0 && medida1 < 30);
  if (estadoAtual1 != ultimoEstado1) {
    const char* mensagem = estadoAtual1 ? "1" : "0";
    brokerMqtt.publish(TOPIC5, mensagem);
    Serial.print(">>> SENSOR 1 (TOPIC5): "); Serial.println(mensagem);
    ultimoEstado1 = estadoAtual1;
  }
  
  bool estadoAtual2 = (medida2 > 0 && medida2 < 30);
  if (estadoAtual2 != ultimoEstado2) {
    const char* mensagem = estadoAtual2 ? "1" : "0";
    brokerMqtt.publish(TOPIC6, mensagem);
    Serial.print(">>> SENSOR 2 (TOPIC6): "); Serial.println(mensagem);
    ultimoEstado2 = estadoAtual2;
  }
  
  if (estadoAtual1 || estadoAtual2) {
    alterarLed(255, 255, 0);
  } else {
    alterarLed(0, 255, 0);
  }
  
  Serial.println("-------------------------------");
  Serial.print("Medicao 1 (TOPIC5): "); Serial.print(medida1); Serial.println(" cm");
  Serial.print("Medicao 2 (TOPIC6): "); Serial.print(medida2); Serial.println(" cm");
  Serial.println("-------------------------------");
  
  delay(1000);
}
