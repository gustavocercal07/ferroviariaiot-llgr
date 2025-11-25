// Bibliotecas necessárias para WiFi, MQTT e sensor DHT
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "env.h" // Arquivo que guarda senhas e informações sensíveis (SSID, senhas, broker, etc.)

// Definições dos pinos dos sensores
#define PINO_LDR 34       // Sensor de luminosidade (LDR)
#define PINO_DHT 4        // Sensor de temperatura e umidade (DHT)
#define PINO_TRIGGER 22   // Pino de envio do sinal ultrassônico
#define PINO_ECHO 23      // Pino de recepção do sinal ultrassônico
#define TIPO_DHT DHT11    // Tipo do sensor DHT

// Pinos do LED RGB
const byte pinoVermelho = 14;
const byte pinoVerde = 26;
const byte pinoAzul = 25;

// Criação dos objetos dos sensores e comunicação
DHT sensorDHT(PINO_DHT, TIPO_DHT);
WiFiClientSecure clienteWifi;
PubSubClient clienteMqtt(clienteWifi);

// Variáveis de controle
int distanciaAtual = 0;
bool estadoPresencaAnterior = false;

// Função para definir a cor do LED RGB
void definirCorLed(byte r, byte g, byte b) {
  ledcWrite(pinoVermelho, r);
  ledcWrite(pinoVerde, g);
  ledcWrite(pinoAzul, b);
}

// Função para medir a distância com o sensor ultrassônico
int medirDistancia(byte pino_echo, byte pino_trigger) {
  digitalWrite(pino_trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pino_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pino_trigger, LOW);
  
  unsigned long duracao = pulseIn(pino_echo, HIGH, 30000); // Mede o tempo do eco
  if (duracao == 0) return -1; // Caso não haja retorno
  return (duracao * 0.0343) / 2; // Converte tempo em distância (cm)
}

// Função para conectar o ESP32 ao Wi-Fi
void conectarWifi() {
  Serial.print("Iniciando conexao WiFi: ");
  Serial.println(SSID);
  definirCorLed(0, 0, 255); // LED azul = tentando conectar
  WiFi.begin(SSID, PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Conectado com sucesso
  Serial.println("\nWiFi conectado com sucesso!");
  Serial.print("IP obtido: ");
  Serial.println(WiFi.localIP());
  definirCorLed(0, 255, 0); // LED verde = conectado
}

// Função para reconectar ao servidor MQTT se cair a conexão
void reconectarBroker() {
  while (!clienteMqtt.connected()) {
    Serial.print("Estabelecendo conexao MQTT... ");
    if (clienteMqtt.connect("ESP32_S1", BROKER_USER_NAME, BROKER_USER_PASS)) {
      Serial.println("Broker MQTT conectado!");
      definirCorLed(0, 255, 0); // LED verde = conectado
    } else {
      Serial.print("Erro na conexao (codigo=");
      Serial.print(clienteMqtt.state());
      Serial.println("). Nova tentativa em 5s...");
      delay(5000);
    }
  }
}

// Configuração inicial do sistema
void setup() {
  Serial.begin(115200);
  pinMode(PINO_LDR, INPUT);        // Configura LDR como entrada
  sensorDHT.begin();               // Inicializa o DHT
  pinMode(PINO_TRIGGER, OUTPUT);   // Trigger como saída
  pinMode(PINO_ECHO, INPUT);       // Echo como entrada
  
  // Configura PWM dos pinos do LED RGB
  ledcAttach(pinoVermelho, 5000, 8);
  ledcAttach(pinoVerde, 5000, 8);
  ledcAttach(pinoAzul, 5000, 8);
  definirCorLed(255, 0, 0); // LED vermelho = inicializando
  
  conectarWifi(); // Conecta ao Wi-Fi
  clienteWifi.setInsecure(); // Ignora certificados SSL (modo inseguro)
  clienteMqtt.setServer(BROKER_URL, BROKER_PORT); // Configura o servidor MQTT
}

// Loop principal do programa
void loop() {
  // Verifica se o MQTT está conectado
  if (!clienteMqtt.connected()) {
    reconectarBroker();
  }
  clienteMqtt.loop();

  // Lê os valores dos sensores
  int valorLuminosidade = analogRead(PINO_LDR);
  float valorTemperatura = sensorDHT.readTemperature();
  float valorUmidade = sensorDHT.readHumidity();
  
  // Envia os dados para o broker MQTT
  char bufferLuz[10];
  sprintf(bufferLuz, "%d", valorLuminosidade);
  clienteMqtt.publish(TOPIC3, bufferLuz);

  char bufferTemp[10];
  dtostrf(valorTemperatura, 4, 1, bufferTemp);
  clienteMqtt.publish(TOPIC1, bufferTemp);

  char bufferUmi[10];
  dtostrf(valorUmidade, 4, 1, bufferUmi);
  clienteMqtt.publish(TOPIC2, bufferUmi);

  // Mostra os dados no monitor serial
  Serial.println("Dados transmitidos:");
  Serial.print("Luz: "); Serial.println(valorLuminosidade);
  Serial.print("Temp: "); Serial.println(valorTemperatura);
  Serial.print("Umid: "); Serial.println(valorUmidade);
  Serial.println("-------------------------------");
  
  // Mede distância com o sensor ultrassônico
  distanciaAtual = medirDistancia(PINO_ECHO, PINO_TRIGGER);
  if (distanciaAtual > 0) {
    Serial.print("Distancia medida: ");
    Serial.print(distanciaAtual);
    Serial.println(" cm");
  }

  // Verifica se há presença de um objeto até 30 cm de distância
  bool presencaDetectada = (distanciaAtual > 0 && distanciaAtual < 30);
  
  // Só envia mensagem se o estado mudou (evita repetições desnecessárias)
  if (presencaDetectada != estadoPresencaAnterior) {
    if (presencaDetectada) {
      clienteMqtt.publish(TOPIC4, "1");
      Serial.println(">>> OBJETO DETECTADO");
      definirCorLed(255, 255, 0); // LED amarelo = presença
    } else {
      clienteMqtt.publish(TOPIC4, "0");
      Serial.println(">>> OBJETO AUSENTE");
      definirCorLed(0, 255, 0);   // LED verde = ausência
    }
    estadoPresencaAnterior = presencaDetectada;
  }
  
  delay(5000); // Espera 5 segundos antes da próxima leitura
}
