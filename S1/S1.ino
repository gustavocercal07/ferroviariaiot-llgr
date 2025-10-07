#include <WiFi.h>
#include <PubSubClient.h>

//cria objeto p/wifi
WiFiClient client;
//criar objeto p/ mqtt usando WiFi
PubSubClient mqtt(client);

//defini nome da rede
const String SSID = "FIESC_IOT_EDU";
//defini senha da rede
const String PASS = "8120gv08";

//defini endereço do Broker
const String brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;

const String brokerUser = "";  //Variável para o user do broker
const String brokerPass = "";  //Variável para a senha dop broker

void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, PASS);
  Serial.println("Conectando no WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.print("\nConectado com sucesso!");

  //criar um nome que começa com "s1-"
  String bordID = "S1-";
  //junta o "s1-" com um número aleatório Hexadecimal
  bordID += String(random(0xfff), HEX);

  mqtt.setServer(brokerURL.c_str(), brokerPort);
  Serial.println("conectando no Broker");
  //enquanto não estiver conectado mostra"."
  while (!mqtt.connect(bordID.c_str())) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado com sucesso ao broker!");
}

void loop() {
  // put your main code here, to run repeatedly:
}