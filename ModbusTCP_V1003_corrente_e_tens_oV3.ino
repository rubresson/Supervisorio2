// inclusão da biblioteca do sensor/transdutor de sinais
#include <max6675.h> 
// inclusão da biblioteca para comunicação Ethernet
#include <EtherCard.h>
// inclusão da biblioteca IP Modbus apra chip ENC28J60 
#include <ModbusIP_ENC28J60.h> 
// inclusão da biblioteca base do protocolo Modbus
#include <Modbus.h> 

 // tempo das leituras millisecundos
#define REQUEST_RATE 1000

// ethernet interface ip address
#define STATIC 1  // //Para definir um IP estático mudar de 0 para 1

#if STATIC
// Endereço IP da interface ethernet do arduino
static byte myip[] = { 192, 168, 100, 10};
// Endereço IP do Gateway da rede
static byte gwip[] = { 192, 168, 100, 1 };
#endif

//O endereço de controle de acesso à mídia (hardware ethernet) para o escudo
static byte mymac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };

//Configuração do registro Modbus para representar
//os valores das variáveis.
//Este valor é o deslocamento (baseado em 0) a ser colocado//
//em seu software de supervisão.
//o software usa deslocamentos baseados em 1, o valor definido deve ser 101.
const int SENSOR_IREG_CORRENTE = 100;
const int SENSOR_IREG_LUMINO1 = 101;
const int SENSOR_IREG_LUMINO2 = 102;
const int SENSOR_IREG_LUMINO3 = 103;
const int SENSOR_IREG_LUMINO4 = 104;
const int SENSOR_IREG_VOLTAGE = 105;
const int SENSOR_IREG_TEMP = 106;

//Pinos de leitura usados
const int sensorPin = A0; //sensor de corrente e voltage
const int sensorPin1 = A1; //sensor de luminosidade
const int sensorPin2 = A2; //sensor de luminosidade
const int sensorPin3 = A3; //sensor de luminosidade
const int sensorPin4 = A4; //sensor de luminosidade
const int sensorPin5 = A5; //sensor de temperatura

//Variáveis tensão e corrente
int mVperAmp = 66;//Constante que converte mV para mA. Usa 100 para o módulo de 20A e 66 para o módulo de 30A
int RawValue = 0;// Guardar o valor em PCM discretizado na entrada analógica
int ACSoffset = 2500;//ajuste de offset. Demonstra o sinal da corrente se é negativa ou positiva
double Voltage = 0;//Realiza a leitura da tensão em PCM (mV para mA)
double Amps = 0;//Armazena o valor já discretizado e devidamente calculado

//Pinos usados no termopar com a identificação das respectivas entradas digitais do arduino
int thermoSO = 3;//Atribui o pino 3 como SO
int thermoCS = 4;//Atribui o pino 4 como CS
int thermoSCK = 5;//Atribui 5 pino 5 como SCK SO


//cria o objeto para comunicação com sensor temopar e seta os pinos digitais
MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);

ModbusIP mb; //Crie a instância mb (ModbusIP) a ser usada.

long ts;

void setup()
{
  mb.config(mymac, myip); // configura IP Modbus
 // Adiciona registrador tipo registro de entrada analógicas
  mb.addIreg(SENSOR_IREG_CORRENTE);
  mb.addIreg(SENSOR_IREG_VOLTAGE);
  mb.addIreg(SENSOR_IREG_LUMINO1);
  mb.addIreg(SENSOR_IREG_LUMINO2);
  mb.addIreg(SENSOR_IREG_LUMINO3);
  mb.addIreg(SENSOR_IREG_LUMINO4);
  mb.addIreg(SENSOR_IREG_TEMP);
}

void loop()
{
  //Roda a tarefa do modbus. Possibilita a conectividade fazendo a leitura com o dispositivo.
  //Respondendo requisições e alterando os registradores se necessário, devendo ser chamado apenas uma vez, logo no início do loop.
  mb.task();
  
  Calcula_corrente_voltage();
  
  if (millis() > ts + 300) {
    ts = millis();
    
    //Realiza o valor do registro da variáveis analógicas
    //definidas em mb.addIreg para acioná-la.
    mb.Ireg(SENSOR_IREG_CORRENTE, analogRead(Amps));
    mb.Ireg(SENSOR_IREG_VOLTAGE, analogRead(Voltage));
    mb.Ireg(SENSOR_IREG_LUMINO1, analogRead(sensorPin1));
    mb.Ireg(SENSOR_IREG_LUMINO2, analogRead(sensorPin2));
    mb.Ireg(SENSOR_IREG_LUMINO3, analogRead(sensorPin3));
    mb.Ireg(SENSOR_IREG_LUMINO4, analogRead(sensorPin4));
    mb.Ireg(SENSOR_IREG_TEMP, thermocouple.readCelsius());//Lê a leitura do temopar em Celsius
   }
}
void Calcula_corrente_voltage()
{ //calcula a corrente e voltagem
  RawValue = analogRead(sensorPin);
  Voltage = (RawValue / 1024.0) * 5000;// Converte PCM para mV 
  Amps = ((Voltage - ACSoffset) / mVperAmp);// Converte mV para o relativo em corrente
}
