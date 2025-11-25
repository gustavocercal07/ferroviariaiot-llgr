// BIBLIOTECAS - ferramentas que o código usa
#include <WiFi.h>              // WiFi
#include <PubSubClient.h>      // MQTT (mensagens)
#include <WiFiClientSecure.h>  // Conexão segura
#include "env.h"               // Senhas e configurações

// Cria conexão WiFi e MQTT
WiFiClientSecure clienteSeguro;
PubSubClient mqtt(clienteSeguro);

// FUNÇÃO QUE RECEBE MENSAGENS
void processarMensagem(char* topico, byte* conteudo, unsigned long tamanho) {
  // Converte mensagem para texto
  String textoRecebido = "";
  for(int i = 0; i < tamanho; i++) {
    textoRecebido += (char) conteudo[i];
  }
  Serial.println(textoRecebido);

  if(textoRecebido == "1") {
    digitalWrite(2, HIGH);
    Serial.println("Ativando saida...");
  }
  if(textoRecebido == "0") {
    digitalWrite(2, LOW);
    Serial.println("Desativando saida...");
  }
}

// SETUP - roda 1 vez quando liga
void setup() {
  Serial.begin(115200);  // Inicia comunicação serial

  // CONECTA NO WIFI
  clienteSeguro.setInsecure();
  WiFi.begin(SSID, PASS);

  Serial.println("Iniciando conexao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("WiFi conectado");

  // CONECTA NO BROKER MQTT
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  Serial.println("Conectando ao broker");

  // Cria ID único para a placa
  String idDispositivo = "S3-";
  idDispositivo += String(random(0xffff), HEX);

  while (!mqtt.connected()) {
    mqtt.connect(idDispositivo.c_str(), BROKER_USER_NAME, BROKER_USER_PASS);
    Serial.print(".");
    delay(200);
  }
  Serial.println("Broker conectado");

  // Inscreve nos tópicos 
  mqtt.subscribe(TOPIC4);
  mqtt.subscribe(TOPIC5);
  mqtt.subscribe(TOPIC6);
  mqtt.setCallback(processarMensagem);  // Define função que processa mensagens

  pinMode(2, OUTPUT);  // Pino 2 como saída (LED)
}

// LOOP - roda infinitamente
void loop() {
  // Envia mensagem manual
  String dadosSerial = "";
  if (Serial.available() > 0) {
    dadosSerial = Serial.readStringUntil('\n');
    Serial.print("Texto digitado: ");
    Serial.println(dadosSerial);
    mqtt.publish("llgr", dadosSerial.c_str());
  }
  mqtt.loop();  // Mantém MQTT funcionando
}
