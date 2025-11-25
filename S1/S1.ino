#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "env.h"

#define PINO_LDR 34
#define PINO_DHT 4
#define PINO_TRIGGER 22
#define PINO_ECHO 23
#define TIPO_DHT DHT11

const byte pinoVermelho = 14;
const byte pinoVerde = 26;
const byte pinoAzul = 25;

DHT sensorDHT(PINO_DHT, TIPO_DHT);
WiFiClientSecure clienteWifi;
PubSubClient clienteMqtt(clienteWifi);

int distanciaAtual = 0;
bool estadoPresencaAnterior = false;

void definirCorLed(byte r, byte g, byte b) {
  ledcWrite(pinoVermelho, r);
  ledcWrite(pinoVerde, g);
  ledcWrite(pinoAzul, b);
}

int medirDistancia(byte pino_echo, byte pino_trigger) {
  digitalWrite(pino_trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pino_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pino_trigger, LOW);
  
  unsigned long duracao = pulseIn(pino_echo, HIGH, 30000);
  if (duracao == 0) return -1;
  return (duracao * 0.0343) / 2;
}

void conectarWifi() {
  Serial.print("Iniciando conexao WiFi: ");
  Serial.println(SSID);
  definirCorLed(0, 0, 255);
  WiFi.begin(SSID, PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi conectado com sucesso!");
  Serial.print("IP obtido: ");
  Serial.println(WiFi.localIP());
  definirCorLed(0, 255, 0);
}

void reconectarBroker() {
  while (!clienteMqtt.connected()) {
    Serial.print("Estabelecendo conexao MQTT... ");
    if (clienteMqtt.connect("ESP32_S1", BROKER_USER_NAME, BROKER_USER_PASS)) {
      Serial.println("Broker MQTT conectado!");
      definirCorLed(0, 255, 0);
    } else {
      Serial.print("Erro na conexao (codigo=");
      Serial.print(clienteMqtt.state());
      Serial.println("). Nova tentativa em 5s...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_LDR, INPUT);
  sensorDHT.begin();
  pinMode(PINO_TRIGGER, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  
  ledcAttach(pinoVermelho, 5000, 8);
  ledcAttach(pinoVerde, 5000, 8);
  ledcAttach(pinoAzul, 5000, 8);
  definirCorLed(255, 0, 0);
  
  conectarWifi();
  clienteWifi.setInsecure();
  clienteMqtt.setServer(BROKER_URL, BROKER_PORT);
}

void loop() {
  if (!clienteMqtt.connected()) {
    reconectarBroker();
  }
  clienteMqtt.loop();
  
  int valorLuminosidade = analogRead(PINO_LDR);
  float valorTemperatura = sensorDHT.readTemperature();
  float valorUmidade = sensorDHT.readHumidity();
  
  char bufferLuz[10];
  sprintf(bufferLuz, "%d", valorLuminosidade);
  clienteMqtt.publish(TOPIC3, bufferLuz);
  
  char bufferTemp[10];
  dtostrf(valorTemperatura, 4, 1, bufferTemp);
  clienteMqtt.publish(TOPIC1, bufferTemp);
  
  char bufferUmi[10];
  dtostrf(valorUmidade, 4, 1, bufferUmi);
  clienteMqtt.publish(TOPIC2, bufferUmi);
  
  Serial.println("Dados transmitidos:");
  Serial.print("Luz: "); Serial.println(valorLuminosidade);
  Serial.print("Temp: "); Serial.println(valorTemperatura);
  Serial.print("Umid: "); Serial.println(valorUmidade);
  Serial.println("-------------------------------");
  
  distanciaAtual = medirDistancia(PINO_ECHO, PINO_TRIGGER);
  if (distanciaAtual > 0) {
    Serial.print("Distancia medida: ");
    Serial.print(distanciaAtual);
    Serial.println(" cm");
  }
  
  bool presencaDetectada = (distanciaAtual > 0 && distanciaAtual < 30);
  
  if (presencaDetectada != estadoPresencaAnterior) {
    if (presencaDetectada) {
      clienteMqtt.publish(TOPIC4, "1");
      Serial.println(">>> OBJETO DETECTADO");
      definirCorLed(255, 255, 0);
    } else {
      clienteMqtt.publish(TOPIC4, "0");
      Serial.println(">>> OBJETO AUSENTE");
      definirCorLed(0, 255, 0);
    }
    estadoPresencaAnterior = presencaDetectada;
  }
  
  delay(5000);
}
