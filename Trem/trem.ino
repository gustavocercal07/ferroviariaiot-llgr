#include <WiFi.h>                
#include <PubSubClient.h>        
#include <WiFiClientSecure.h>    
#include "env.h"                 

WiFiClientSecure clienteWifi;      // Cliente WiFi seguro
PubSubClient clienteMqtt(clienteWifi); // Cliente MQTT

// Pinos dos LEDs
const int PINO_LED_VERDE = 18;
const int PINO_LED_VERMELHO = 19;


// Função chamada quando chega mensagem MQTT
void tratarMensagemRecebida(char* topico, byte* payload, unsigned long tamanho) {
  
  String mensagemCompleta = "";

  // Monta a mensagem recebida
  for(int i = 0; i < tamanho; i++) {
    mensagemCompleta += (char) payload[i];
  }

  mensagemCompleta.trim();
  Serial.println(mensagemCompleta);

  int valorVelocidade = mensagemCompleta.toInt(); // Converte para número

  Serial.print("Velocidade: ");
  Serial.println(valorVelocidade);

  // Controle dos LEDs
  if (valorVelocidade == 0) {
    digitalWrite(PINO_LED_VERDE, LOW);
    digitalWrite(PINO_LED_VERMELHO, LOW);
  } else if (valorVelocidade > 0) {
    digitalWrite(PINO_LED_VERDE, HIGH);
    digitalWrite(PINO_LED_VERMELHO, LOW);
  } else {
    digitalWrite(PINO_LED_VERDE, LOW);
    digitalWrite(PINO_LED_VERMELHO, HIGH);
  }
}


void setup() {
  Serial.begin(115200);

  clienteWifi.setInsecure(); // Ignora certificado

  // Conecta ao WiFi
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }

  // Configura o broker MQTT
  clienteMqtt.setServer(BROKER_URL, BROKER_PORT);

  // Gera ID do cliente MQTT
  String identificacaoUnica = "Motor-";
  identificacaoUnica += String(random(0xffff), HEX);

  // Conecta ao broker
  while (!clienteMqtt.connected()) {
    clienteMqtt.connect(
      identificacaoUnica.c_str(),
      BROKER_USER_NAME,
      BROKER_USER_PASS
    );
    delay(200);
  }

  // Assina o tópico
  clienteMqtt.subscribe(TOPIC11);

  // Define função de callback
  clienteMqtt.setCallback(tratarMensagemRecebida);

  // Configura LEDs
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  digitalWrite(PINO_LED_VERDE, LOW);
  digitalWrite(PINO_LED_VERMELHO, LOW);
}


void loop() {

  // Envia mensagens digitadas no Serial para o MQTT
  if (Serial.available() > 0) {
    String entradaSerial = Serial.readStringUntil('\n');
    clienteMqtt.publish("bezinho", entradaSerial.c_str());
  }

  // Mantém MQTT funcionando
  clienteMqtt.loop();
}
