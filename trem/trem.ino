#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h> 
#include "env.h"

WiFiClientSecure clienteWifi;
PubSubClient clienteMqtt(clienteWifi);

const int PINO_LED_VERDE = 18;
const int PINO_LED_VERMELHO = 19;

void tratarMensagemRecebida(char* topico, byte* payload, unsigned long tamanho) {
  String mensagemCompleta = "";
  for(int i = 0; i < tamanho; i++) {
    mensagemCompleta += (char) payload[i];
  }
  mensagemCompleta.trim();
  Serial.println(mensagemCompleta);
  
  int valorVelocidade = mensagemCompleta.toInt();
  
  Serial.print("Velocidade obtida: ");
  Serial.println(valorVelocidade);
  
  if (valorVelocidade == 0) {
    digitalWrite(PINO_LED_VERDE, LOW);
    digitalWrite(PINO_LED_VERMELHO, LOW);
    Serial.println("Parado. Ambos LEDs desligados.");
  } else if (valorVelocidade > 0) {
    digitalWrite(PINO_LED_VERDE, HIGH);
    digitalWrite(PINO_LED_VERMELHO, LOW);
    Serial.println("Movimento adiante. LED verde ativo.");
  } else {
    digitalWrite(PINO_LED_VERDE, LOW);
    digitalWrite(PINO_LED_VERMELHO, HIGH);
    Serial.println("Movimento reverso. LED vermelho ativo.");
  }
}

void setup() {
  Serial.begin(115200);
  clienteWifi.setInsecure();
  WiFi.begin(SSID, PASS);
  
  Serial.println("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("WiFi conectado");
  
  clienteMqtt.setServer(BROKER_URL, BROKER_PORT);
  Serial.println("Conectando broker");
  
  String identificacaoUnica = "Motor-";
  identificacaoUnica += String(random(0xffff), HEX);
  
  while (!clienteMqtt.connected()) {
    clienteMqtt.connect(identificacaoUnica.c_str(), BROKER_USER_NAME, BROKER_USER_PASS);
    Serial.print(".");
    delay(200);
  }
  Serial.println("Broker conectado");
  
  clienteMqtt.subscribe(TOPIC11);
  clienteMqtt.setCallback(tratarMensagemRecebida);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  digitalWrite(PINO_LED_VERDE, LOW);
  digitalWrite(PINO_LED_VERMELHO, LOW);
}

void loop() {
  String entradaSerial = "";
  if (Serial.available() > 0) {
    entradaSerial = Serial.readStringUntil('\n');
    Serial.print("Entrada: ");
    Serial.println(entradaSerial);
    clienteMqtt.publish("llgr", entradaSerial.c_str());
  }
  clienteMqtt.loop();
}
