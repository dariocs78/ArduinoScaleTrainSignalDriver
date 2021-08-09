// ======================================================================================================================
// CONTROL DE SEMAFOROS LED DE TRENES A ESCALA CON PLACAS PCA9685 / SCALE-RAILROAD LED SIGNAL DRIVER USING PCA9685 BOARDS
// ======================================================================================================================
// v1.0 - Dario Calvo Sanchez, 2021 - Web: elgajoelegante.com - Twitter: @DarioCalvoS
// Licensed under CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/?ref=chooser-v1)

// - Maximo de 12 semaforos o 48 focos / 12 signals or 48 lights maximum
// - Soporta senales tanto de anodo comun como de catodo comun / Both common-anode and common-cathode configurations are supported
//    + Hasta 4 focos (maximo de 3 encendidos simultaneamente) en anodo comun / Up to 4 lights (max. 3 simultaneously on) in common-anode configuration
//    + Hasta 5 focos (maximo de 5 encendidos simultaneamente) en catodo comun / Up to 5 lights (max. 5 simultaneously on) in common-cathode configuration
// - Compatible con sistemas analogicos y digitales / Compatible with both digital and analogue systems

// VERSION PARA MAQUETAS ANALOGICAS (CONTROL MANUAL) / ANALOGUE LAYOUTS VERSION (MANUAL CONTROL)



//---------------------------------------------------------------------------------------------------------------------------------------------------
// LIBRERIAS / LIBRARIES
//---------------------------------------------------------------------------------------------------------------------------------------------------

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


//---------------------------------------------------------------------------------------------------------------------------------------------------
// VARIABLES
//---------------------------------------------------------------------------------------------------------------------------------------------------

const uint8_t Numero_placas = 3;                                    // Numero total de placas PCA9685 instaladas / Total number of PCA9685 boards installed
const uint8_t Numero_semaforos = 12;                                // Numero total de semaforos instalados (max. 12) / Total number of signals installed (max. 12)
const uint8_t Max_num_focos = 5;                                    // Maximo numero de focos por semaforo
const uint8_t Num_estados_analog =  8;                              // Numero total de aspectos a controlar por pulsadores / Total number of aspects to be controlled by push buttons

const uint16_t frec = 200;                                          // Frecuencia del pulso PWM a los LEDs (Hz) / LED PWM output frequency (Hz)
const uint16_t encendido_ms = 120;                                  // Tiempo de encendido o atenuado (ms) / Dimming time (ms)
unsigned long periodo = 425;                                        // Duracion (en ms) del encendido durante el parpadeo / Blinking time (ms)
const uint16_t pausa_aspectos = 100;                                // Pausa entre aspectos (ms) / Pause between aspects (ms)
unsigned long margen_contador = 5000;
uint16_t espera_cambio_estado = 150;

uint8_t comando[Numero_semaforos];
uint8_t estado[Numero_semaforos];
uint8_t semaforo[Numero_semaforos][Max_num_focos][2];
uint8_t control[Numero_semaforos][Max_num_focos];
bool anodo_comun[Numero_semaforos];
int focos_totales;
int focos_activos[Numero_semaforos];
uint8_t pin_control[Numero_semaforos];
bool pin_dummy[Numero_semaforos];
float factor_brillo[Numero_semaforos];
int limite_ciclo[Numero_semaforos];
int paso_adaptado[Numero_semaforos];

bool proceso_nuevo_comando[Numero_semaforos];
bool candidato_apagado[Numero_semaforos][Max_num_focos];
uint8_t control_apagado_total[Numero_semaforos];
uint16_t contador_cambio_estado[Numero_semaforos];
bool flag_parpadeo[Numero_semaforos][Max_num_focos];
bool en_ciclo[Numero_semaforos][Max_num_focos];
bool flag_tiempo[Numero_semaforos][Max_num_focos];
unsigned long tiempos[Numero_semaforos][Max_num_focos][2];
bool flag_duty;
int duty;
int paso;

uint16_t contador;
bool flag_ciclo;

uint8_t pin_estado[Num_estados_analog];
bool control_de_pulsadores;
uint8_t control_de_semaforos;


struct comando_semaforos
{
   uint8_t estado;
};


Adafruit_PWMServoDriver placa[Numero_placas];

comando_semaforos comando_recibido[Numero_semaforos];



//---------------------------------------------------------------------------------------------------------------------------------------------------
// CONDICIONES INICIALES / INITIAL SETUP
//---------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {

  // PINES PARA CONTROLAR ASPECTOS POR PULSADORES / PINS TO CONTROL ASPECTS USING PUSH BUTTONS

    // El maximo numero de aspectos posible viene dado por el numero de pines libres del Arduino: / Maximum controllable aspect number depends on Arduino's free pins:
    // - Dos pines (18 y 19) estaran siempre ocupados por el bus I2C / Two pins (18 & 19) will be always occupied by I2C connection
    // - Habra tantos pines ocupados como numero de semaforos conectados (maximo 12) / There will be as much occupied pins as signals are connected (max. 12)

    // Rellenar el pin para cada pulsador de estado / Fill-in the pin number for each aspect push-button
    // Los pines analógicos (A0-A7) se numeran del 14 al 21 / Analog pins (A0-A7) take numbers from 14 to 21
    // El total de pines debe coincidir con el valor de "Num_estados_analog" / Total number of pins must match "Num_estados_analog" value

    // Anadir pines en caso necesario / Add pins if required
    // Comentar o borrar los pines no utilizados / Comment or delete unused pins

        pin_estado[0] = 14;     // Pin para el pulsador de aspecto "Parada" / "Stop" aspect control pin
        pin_estado[1] = 15;     // Pin para el pulsador de aspecto "Via libre" / "Proceed" aspect control pin
        pin_estado[2] = 16;     // Pin para el pulsador de aspecto "Via libre condicional" / "Proceed on condition" aspect control pin
        pin_estado[3] = 17;     // Pin para el pulsador de aspecto "Anuncio de precaucion" / "Precaution warning" aspect control pin
        pin_estado[4] = 20;     // Pin para el pulsador de aspecto "Anuncio de parada" / "Stop warning" aspect control pin
        pin_estado[5] = 21;     // Pin para el pulsador de aspecto "Anuncio de parada inmediata" / "Immediate stop warning" aspect control pin
        pin_estado[6] =  0;     // Pin para el pulsador de aspecto "Rebase autorizado (con parada)" / "Authorized pass (stopping on signal)" aspect control pin
        pin_estado[7] =  1;     // Pin para el pulsador de aspecto "Movimiento autorizado" / "Authorized movement" aspect control pin


  // PLACAS PCA9685 / PCA9685 BOARDS SETUP

    // Rellenar la direccion de las placas utilizadas / Fill-in the address for each board connected
    // Comentar placas no utilizadas / Comment unused boards

    // Placa 0 / Board 0
      placa[0] = Adafruit_PWMServoDriver(0x40);         // 0x40 direccion por defecto en estas placas / 0x40 default address
      placa[0].begin();
      placa[0].setPWMFreq(frec);

    // Placa 1 / Board 1
      placa[1] = Adafruit_PWMServoDriver(0x41);
      placa[1].begin();
      placa[1].setPWMFreq(frec);

    // Placa 2 / Board 2
      placa[2] = Adafruit_PWMServoDriver(0x42);
      placa[2].begin();
      placa[2].setPWMFreq(frec);


  // SEMAFOROS / SIGNALS SETUP

    // Rellenar los siguientes parametros para cada semaforo: / Fill-in the following parameters for each signal:

    // - "anodo_comun[a]":    "true" si es un semaforo de anodo comun, "false" si es de catodo comun / "true" if it is a common-anode signal, "false" if it is common-cathode type
    // - "pin_dummy[a]":      sin uso, dejar el valor "false" / not used, leave "false"
    // - "semaforo[a][b][0]": para cada luz de cada semaforo, numero de la placa PCA9685 donde esta conectada esa luz (99 si no se usa esa luz) / for each light on each signal, PCA9685 board number where that light is connected (99 if the light is not used)
    //   "semaforo[a][b][1]": para cada luz de cada semaforo, numero del pin de la placa PCA9685 donde esta conectada esa luz (99 si no se usa esa luz) / for each light on each signal, pin number on the PCA9685 board where that light is connected (99 if the light is not used)
    // - "comando[a]":        aspecto inicial del semaforo (ver lista de aspectos) / initial aspect for this signal (see aspect list)

    //    Donde / Where:
    //    + a: numero del semaforo empezando por 0 (con valor maximo definido por "Numero_semaforos - 1" / signal id, starting from 0 (up to a maximum defined by "Numero_semaforos - 1")
    //    + b: numero del foco, empezando por 0 (con valor maximo definido por "Max_num_focos - 1") / light ID, starting from 0 (up to a maximum defined by "Max_num_focos - 1")


      // Semaforo 0 / Signal 0
      // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[0] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[0] = false;                         // Sin uso / Not in use
          pin_control[0] =  2;                          // Para control analogico / For control in non-digital layouts

          semaforo[0][0][0] =  0;                       // Placa foco verde / Green light board
          semaforo[0][0][1] =  8;                       // Pin foco verde / Green light pin

          semaforo[0][1][0] =  0;                       // Placa foco rojo / Red light board
          semaforo[0][1][1] =  9;                       // Pin foco rojo / Red light pin

          semaforo[0][2][0] =  0;                       // Placa foco amarillo / Yellow light board
          semaforo[0][2][1] = 10;                       // Pin foco amarillo / Yellow light pin

          semaforo[0][3][0] =  0;                       // Placa foco blanco / White light board
          semaforo[0][3][1] = 11;                       // Pin foco blanco / White light pin

          semaforo[0][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[0][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[0] =  13;                             // Aspecto inicial / Initial aspect


    // Semaforo 1 / Signal 1
    // Referencia / Reference: Ejemplo / Example - Mafen 4131.11 (principal con 3 focos)

          anodo_comun[1] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[1] = false;                         // Sin uso / Not in use
          pin_control[1] =  3;                          // Para control analogico / For control in non-digital layouts

          semaforo[1][0][0] =  0;                       // Placa foco verde / Green light board
          semaforo[1][0][1] =  4;                       // Pin foco verde / Green light pin

          semaforo[1][1][0] =  0;                       // Placa foco rojo / Red light board
          semaforo[1][1][1] =  5;                       // Pin foco rojo / Red light pin

          semaforo[1][2][0] =  0;                       // Placa foco amarillo / Yellow light board
          semaforo[1][2][1] =  6;                       // Pin foco amarillo / Yellow light pin

          semaforo[1][3][0] = 99;                       // Placa foco blanco / White light board
          semaforo[1][3][1] = 99;                       // Pin foco blanco / White light pin

          semaforo[1][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[1][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[1] = 2;                               // Aspecto inicial / Initial aspect


      // Semaforo 2 / Signal 2
      // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[2] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[2] = false;                         // Sin uso / Not in use
          pin_control[0] =  4;                          // Para control analogico / For control in non-digital layouts

          semaforo[2][0][0] =  0;                       // Placa foco verde / Green light board
          semaforo[2][0][1] = 12;                       // Pin foco verde / Green light pin

          semaforo[2][1][0] =  0;                       // Placa foco rojo / Red light board
          semaforo[2][1][1] = 13;                       // Pin foco rojo / Red light pin

          semaforo[2][2][0] =  0;                       // Placa foco amarillo / Yellow light board
          semaforo[2][2][1] = 14;                       // Pin foco amarillo / Yellow light pin

          semaforo[2][3][0] =  0;                       // Placa foco blanco / White light board
          semaforo[2][3][1] = 15;                       // Pin foco blanco / White light pin

          semaforo[2][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[2][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[2] = 2;                               // Aspecto inicial / Initial aspect


      // Semaforo 3 / Signal 3
      // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[3] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[3] = false;                         // Sin uso / Not in use
          pin_control[0] =  5;                          // Para control analogico / For control in non-digital layouts

          semaforo[3][0][0] =  0;                       // Placa foco verde / Green light board
          semaforo[3][0][1] =  0;                       // Pin foco verde / Green light pin

          semaforo[3][1][0] =  0;                       // Placa foco rojo / Red light board
          semaforo[3][1][1] =  1;                       // Pin foco rojo / Red light pin

          semaforo[3][2][0] =  0;                       // Placa foco amarillo / Yellow light board
          semaforo[3][2][1] =  2;                       // Pin foco amarillo / Yellow light pin

          semaforo[3][3][0] =  0;                       // Placa foco blanco / White light board
          semaforo[3][3][1] =  3;                       // Pin foco blanco / White light pin

          semaforo[3][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[3][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[3] = 2;                               // Aspecto inicial / Initial aspect


      // Semaforo 4 / Signal 4
      // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[4] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[4] = false;                         // Sin uso / Not in use
          pin_control[0] =  6;                          // Para control analogico / For control in non-digital layouts

          semaforo[4][0][0] =  1;                       // Placa foco verde / Green light board
          semaforo[4][0][1] = 12;                       // Pin foco verde / Green light pin

          semaforo[4][1][0] =  1;                       // Placa foco rojo / Red light board
          semaforo[4][1][1] = 13;                       // Pin foco rojo / Red light pin

          semaforo[4][2][0] =  1;                       // Placa foco amarillo / Yellow light board
          semaforo[4][2][1] = 14;                       // Pin foco amarillo / Yellow light pin

          semaforo[4][3][0] =  1;                       // Placa foco blanco / White light board
          semaforo[4][3][1] = 15;                       // Pin foco blanco / White light pin

          semaforo[4][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[4][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[4] = 2;                               // Aspecto inicial / Initial aspect


      // Semaforo 5 / Signal 5
      // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[5] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[5] = false;                         // Sin uso / Not in use
          pin_control[0] =  7;                          // Para control analogico / For control in non-digital layouts

          semaforo[5][0][0] =  1;                       // Placa foco verde / Green light board
          semaforo[5][0][1] =  0;                       // Pin foco verde / Green light pin

          semaforo[5][1][0] =  1;                       // Placa foco rojo / Red light board
          semaforo[5][1][1] =  1;                       // Pin foco rojo / Red light pin

          semaforo[5][2][0] =  1;                       // Placa foco amarillo / Yellow light board
          semaforo[5][2][1] =  2;                       // Pin foco amarillo / Yellow light pin

          semaforo[5][3][0] =  1;                       // Placa foco blanco / White light board
          semaforo[5][3][1] =  3;                       // Pin foco blanco / White light pin

          semaforo[5][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[5][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[5] = 2;                               // Aspecto inicial / Initial aspect


      // Semaforo 6 / Signal 6
      // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[6] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[6] = false;                         // Sin uso / Not in use
          pin_control[0] =  8;                          // Para control analogico / For control in non-digital layouts

          semaforo[6][0][0] =  1;                       // Placa foco verde / Green light board
          semaforo[6][0][1] =  8;                       // Pin foco verde / Green light pin

          semaforo[6][1][0] =  1;                       // Placa foco rojo / Red light board
          semaforo[6][1][1] =  9;                       // Pin foco rojo / Red light pin

          semaforo[6][2][0] =  1;                       // Placa foco amarillo / Yellow light board
          semaforo[6][2][1] = 10;                       // Pin foco amarillo / Yellow light pin

          semaforo[6][3][0] =  1;                       // Placa foco blanco / White light board
          semaforo[6][3][1] = 11;                       // Pin foco blanco / White light pin

          semaforo[6][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[6][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[6] = 2;                               // Aspecto inicial / Initial aspect


    // Semaforo 7 / Signal 7
    // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[7] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[7] = false;                         // Sin uso / Not in use
          pin_control[0] =  9;                          // Para control analogico / For control in non-digital layouts

          semaforo[7][0][0] =  1;                       // Placa foco verde / Green light board
          semaforo[7][0][1] =  4;                       // Pin foco verde / Green light pin

          semaforo[7][1][0] =  1;                       // Placa foco rojo / Red light board
          semaforo[7][1][1] =  5;                       // Pin foco rojo / Red light pin

          semaforo[7][2][0] =  1;                       // Placa foco amarillo / Yellow light board
          semaforo[7][2][1] =  6;                       // Pin foco amarillo / Yellow light pin

          semaforo[7][3][0] =  1;                       // Placa foco blanco / White light board
          semaforo[7][3][1] =  7;                       // Pin foco blanco / White light pin

          semaforo[7][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[7][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[7] = 2;                               // Aspecto inicial / Initial aspect


    // Semaforo 8 / Signal 8
    // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[8] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[8] = false;                         // Sin uso / Not in use
          pin_control[0] = 10;                          // Para control analogico / For control in non-digital layouts

          semaforo[8][0][0] =  2;                       // Placa foco verde / Green light board
          semaforo[8][0][1] =  4;                       // Pin foco verde / Green light pin

          semaforo[8][1][0] =  2;                       // Placa foco rojo / Red light board
          semaforo[8][1][1] =  5;                       // Pin foco rojo / Red light pin

          semaforo[8][2][0] =  2;                       // Placa foco amarillo / Yellow light board
          semaforo[8][2][1] =  6;                       // Pin foco amarillo / Yellow light pin

          semaforo[8][3][0] =  2;                       // Placa foco blanco / White light board
          semaforo[8][3][1] =  7;                       // Pin foco blanco / White light pin

          semaforo[8][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[8][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[8] = 2;                               // Aspecto inicial / Initial aspect


    // Semaforo 9 / Signal 9
    // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[9] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[9] = false;                         // Sin uso / Not in use
          pin_control[0] = 11;                          // Para control analogico / For control in non-digital layouts

          semaforo[9][0][0] =  2;                       // Placa foco verde / Green light board
          semaforo[9][0][1] = 12;                       // Pin foco verde / Green light pin

          semaforo[9][1][0] =  2;                       // Placa foco rojo / Red light board
          semaforo[9][1][1] = 13;                       // Pin foco rojo / Red light pin

          semaforo[9][2][0] =  2;                       // Placa foco amarillo / Yellow light board
          semaforo[9][2][1] = 14;                       // Pin foco amarillo / Yellow light pin

          semaforo[9][3][0] =  2;                       // Placa foco blanco / White light board
          semaforo[9][3][1] = 15;                       // Pin foco blanco / White light pin

          semaforo[9][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[9][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[9] = 2;                               // Aspecto inicial / Initial aspect


    // Semaforo 10 / Signal 10
    // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[10] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[10] = false;                         // Sin uso / Not in use
          pin_control[0] = 12;                           // Para control analogico / For control in non-digital layouts

          semaforo[10][0][0] =  2;                       // Placa foco verde / Green light board
          semaforo[10][0][1] =  0;                       // Pin foco verde / Green light pin

          semaforo[10][1][0] =  2;                       // Placa foco rojo / Red light board
          semaforo[10][1][1] =  1;                       // Pin foco rojo / Red light pin

          semaforo[10][2][0] =  2;                       // Placa foco amarillo / Yellow light board
          semaforo[10][2][1] =  2;                       // Pin foco amarillo / Yellow light pin

          semaforo[10][3][0] =  2;                       // Placa foco blanco / White light board
          semaforo[10][3][1] =  3;                       // Pin foco blanco / White light pin

          semaforo[10][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[10][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[10] = 2;                               // Aspecto inicial / Initial aspect


    // Semaforo 11 / Signal 11
    // Referencia / Reference: Ejemplo / Example - Mafen 4131.15 (de salida con 4 focos)

          anodo_comun[11] = true;                        // Tipo (anodo -true- o catodo -false- comun) / Type (common anode -true- or cathode -false-)
          pin_dummy[11] = false;                         // Sin uso / Not in use
          pin_control[0] = 13;                           // Para control analogico / For control in non-digital layouts

          semaforo[11][0][0] =  2;                       // Placa foco verde / Green light board
          semaforo[11][0][1] =  8;                       // Pin foco verde / Green light pin

          semaforo[11][1][0] =  2;                       // Placa foco rojo / Red light board
          semaforo[11][1][1] =  9;                       // Pin foco rojo / Red light pin

          semaforo[11][2][0] =  2;                       // Placa foco amarillo / Yellow light board
          semaforo[11][2][1] = 10;                       // Pin foco amarillo / Yellow light pin

          semaforo[11][3][0] =  2;                       // Placa foco blanco / White light board
          semaforo[11][3][1] = 11;                       // Pin foco blanco / White light pin

          semaforo[11][4][0] = 99;                       // Placa foco azul / Blue light board
          semaforo[11][4][1] = 99;                       // Pin foco azul / Blue light pin

          comando[11] = 2;                               // Aspecto inicial / Initial aspect


    // Variables iniciales de control y contadores para cada foco y aspecto de cada semaforo / Control variables & counters for each light and aspect

          flag_ciclo = false;
          flag_duty = false;
          duty = 0;
          contador = 0;
          focos_totales = 0;

          control_de_pulsadores = false;
          control_de_semaforos = 255;

          for (uint8_t i = 0; i < Numero_semaforos; i++)
          {
            proceso_nuevo_comando[i] = false;

            focos_activos[i] = 0;
            factor_brillo[i] = 1.0;
            limite_ciclo[i] = 4095;
            paso_adaptado[i] = 0;

            for (uint8_t j = 0; j < Max_num_focos; j++)
            {
              control[i][j] = 0;
              flag_parpadeo[i][j] = false;
              flag_tiempo[i][j] = false;
              en_ciclo[i][j] = false;
              candidato_apagado[i][j] = false;

              if (semaforo[i][j][0] != 99)
              {
                focos_totales++;
              }

            }

          }


  // COMUNICACIONES / COMMUNICATIONS

    // Serial.begin(115200);   // Ha de ser igual al emisor / Must match emitter's rate


  // PARAMETROS DE ATENUACION / DIMMING PARAMETERS

    paso = round(2560.0 * float(focos_totales) / float(encendido_ms));


  // ASPECTO INICIAL Y CONFIGURACION DE PINES / INITIAL ASPECT & PIN CONFIGURATION

    for (uint8_t k = 0; k < Num_estados_analog; k++)
    {
      pinMode(pin_estado[k], INPUT_PULLUP);   // Configuracion de los pines de control de aspecto / Aspect control pin setup
      digitalWrite(pin_estado[k], HIGH);
    }

    for (uint8_t i = 0; i < Numero_semaforos; i++)
    {
      nuevo_comando_ini(i);
      pinMode(pin_control[i], INPUT_PULLUP);  // Configuracion de los pines de seleccion de los semaforos / Signal selection pin setup
      digitalWrite(pin_control[i], HIGH);
    }

}


//---------------------------------------------------------------------------------------------------------------------------------------------------
// BUCLE PRINCIPAL / MAIN LOOP
//---------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {


  // Control del contador / Counter control

    if (contador * paso >= 4095 && flag_ciclo == false)
    {
        contador = 0;
        flag_ciclo = true;
    }


  // Lectura de los pulsadores / Push buttons input

    for (uint8_t i = 0; i < Numero_semaforos; i++)
    {
      if (digitalRead(pin_control[i]) == LOW && control_de_pulsadores == false)
      {
        control_de_pulsadores = true;
        control_de_semaforos = i;
      }
    }

    for (uint8_t k = 0; k < Num_estados_analog; k++)
    {
      if (digitalRead(pin_estado[k]) == LOW && control_de_pulsadores == true)   // Si ya se ha pulsado el boton de un semaforo, faltaria asignar el estado
      {
        comando[control_de_semaforos] = diccionario_pulsador_estado(k);
        control_de_pulsadores = false;
        control_de_semaforos = 255;
      }
    }


  // Control de los semaforos / Signal control

    for (uint8_t i = 0; i < Numero_semaforos; i++)
    {
      // Control de estado actual / Current state control

      lanza_controles(i);

      // Nuevo comando / New command

      if (proceso_nuevo_comando[i] == true)
      {
        contador_cambio_estado[i]++;

        if (contador_cambio_estado[i] == espera_cambio_estado)
        {
          nuevo_comando_2(i);
        }
      }

      if (comando[i] != estado[i])
      {
          proceso_nuevo_comando[i] = false;
          contador_cambio_estado[i] = 0;
          nuevo_comando_1(i);
      }

    }

  // Reinicio del contador / Counter reset

  if (flag_ciclo == true)
  {
    flag_ciclo = false;
  }

  // Incremento del contador / Counter increase

  contador++;


}


//---------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCIONES / FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------------

unsigned long toma_tiempo(uint8_t condicion)                      // Funcion que toma el tiempo de maquina en el instante de ejecucion / Function for timing measurements
{
  unsigned long tempTiempo = millis();

  if (condicion == 0)                                         // Control de seguridad en caso de alcanzar el limite del reloj del sistema / Safety feature in case of being close to reach the system clock limit
  {
      if (tempTiempo >= (4294967295 - margen_contador))
          {
          delay(margen_contador + 500);
          tempTiempo = millis();
          return tempTiempo;
          }
      else
          {
          return tempTiempo;
          }
  }
  else
      {
      return tempTiempo;
      }

}

void lanza_controles(uint8_t indice)                              // Funcion para mantener los semaforos funcionando si no hay cambio de estado / Function to keep signals working
{
  for (uint8_t j = 0; j < Max_num_focos; j++)
  {
    if (semaforo[indice][j][0] != 99)
    {
      switch (control[indice][j])
      {

        // Apagado / Off

        case 0:

          placa[semaforo[indice][j][0]].setPin(semaforo[indice][j][1], 0, anodo_comun[indice]);

          if (flag_parpadeo[indice][j] == true)
          {
            tiempos[indice][j][1] = toma_tiempo(1);

            if (tiempos[indice][j][1] >= tiempos[indice][j][0] + periodo)
            {
              control[indice][j] = 5;
            }
          }

          break;



        // Encendido progresivo / Progressive turn-on

        case 5:

          duty = contador * paso_adaptado[indice];

          if (flag_ciclo == true && en_ciclo[indice][j] == false)
          {
            en_ciclo[indice][j] = true;
          }

          else if (flag_ciclo == false && en_ciclo[indice][j] == true)
          {

            flag_duty = true;
          }

          else if (flag_ciclo == true && en_ciclo[indice][j] == true)
          {
            if (candidato_apagado[indice][j] == true)
            {
              control[indice][j] = 10;
              candidato_apagado[indice][j] = false;
            }
            else
            {
              control[indice][j] = 6;
            }

            en_ciclo[indice][j] = false;

            if (flag_parpadeo[indice][j] == true)
            {
              flag_tiempo[indice][j] = true;
            }

          }


          if (flag_duty == true)
          {
            if (duty <= limite_ciclo[indice])
            {
              placa[semaforo[indice][j][0]].setPin(semaforo[indice][j][1], duty, anodo_comun[indice]);
              flag_duty = false;
            }
            else if (duty > limite_ciclo[indice])
            {
              placa[semaforo[indice][j][0]].setPin(semaforo[indice][j][1], limite_ciclo[indice], anodo_comun[indice]);
              flag_duty = false;
            }
          }

          if (flag_tiempo[indice][j] == true)
          {
            tiempos[indice][j][0] = toma_tiempo(0);
            flag_tiempo[indice][j] = false;
          }

          break;



        // Encendido / On

        case 6:

          placa[semaforo[indice][j][0]].setPin(semaforo[indice][j][1], limite_ciclo[indice], anodo_comun[indice]);

          if (flag_parpadeo[indice][j] == true)
          {
            tiempos[indice][j][1] = toma_tiempo(1);

            if (tiempos[indice][j][1] >= tiempos[indice][j][0] + periodo)
            {
              control[indice][j] = 10;
            }
          }

          break;



        // Atenuado para apagado / Dimming

        case 10:

          duty = limite_ciclo[indice] - (contador * paso_adaptado[indice]);

          if (flag_ciclo == true && en_ciclo[indice][j] == false)
          {
            en_ciclo[indice][j] = true;
          }

          else if (flag_ciclo == false && en_ciclo[indice][j] == true)
          {
            flag_duty = true;
          }

          else if (flag_ciclo == true && en_ciclo[indice][j] == true)
          {
            control[indice][j] = 0;
            en_ciclo[indice][j] = false;

            if (flag_parpadeo[indice][j] == true)
            {
              flag_tiempo[indice][j] = true;
            }

          }


          if (flag_duty == true)
          {
            if (duty >= 0)
            {
              placa[semaforo[indice][j][0]].setPin(semaforo[indice][j][1], duty, anodo_comun[indice]);
              flag_duty = false;
            }
            else if (duty < 0)
            {
              placa[semaforo[indice][j][0]].setPin(semaforo[indice][j][1], 0, anodo_comun[indice]);
              flag_duty = false;
            }
          }

          if (flag_tiempo[indice][j] == true)
          {
            tiempos[indice][j][0] = toma_tiempo(0);
            flag_tiempo[indice][j] = false;
          }

          break;

      }

    }
  }
}

void nuevo_comando_ini(uint8_t indice)                            // Funcion para la configuracion inicial / Function for initial setup
{
  estado[indice] = comando[indice];

  // Asegura el apagado inicial de todos los focos / Ensures all lights are off

  for (uint8_t j = 0; j < Max_num_focos; j++)
  {
    if (semaforo[indice][j][0] != 99)
    {
      placa[semaforo[indice][j][0]].setPin(semaforo[indice][j][1], 0, anodo_comun[indice]);

    }

  }

  unsigned long pausa_0 = millis();
  unsigned long pausa_1 = millis();

  while (pausa_1 <= pausa_0 + 500)
  {
    pausa_1 = millis();
  }

  nuevo_estado(indice);

}

void nuevo_comando_1(uint8_t indice)                              // Funcion para reconfigurar el semaforo si hay cambio de estado (parte 1) / Function for configuring a new aspect (part 1)
{
    estado[indice] = comando[indice];

    // Asegura el apagado inicial de todos los focos / Ensures all lights are off

    control_apagado_total[indice] = 0;

    for (uint8_t j = 0; j < Max_num_focos; j++)
    {
      if (semaforo[indice][j][0] != 99)
      {
        flag_parpadeo[indice][j] = false;
        control_apagado_total[indice]++;

        if (control[indice][j] == 6)
        {
          control[indice][j] = 10;
        }

        if (control[indice][j] == 5)
        {
          candidato_apagado[indice][j] = true;
        }
      }
    }

    proceso_nuevo_comando[indice] = true;
    contador_cambio_estado[indice] = 0;

}

void nuevo_comando_2(uint8_t indice)                              // Funcion para reconfigurar el semaforo si hay cambio de estado (parte 2) / Function for configuring a new aspect (part 2)
{
  for (uint8_t j = 0; j < Max_num_focos; j++)
  {
    if (semaforo[indice][j][0] != 99)
    {
      if (control[indice][j] == 0)
      {
        control_apagado_total[indice] = control_apagado_total[indice] - 1;
      }

    }
  }

  if (control_apagado_total[indice] == 0)
  {
      proceso_nuevo_comando[indice] = false;

      // Espera un poco para hacer el cambio mas estetico / Waits a little to make the change more visually attractive

      unsigned long pausa_0 = millis();
      unsigned long pausa_1 = millis();

      while (pausa_1 <= pausa_0 + pausa_aspectos)
      {
          pausa_1 = millis();
      }

      contador_cambio_estado[indice] = 0;
      nuevo_estado(indice);
  }

}

void nuevo_estado(uint8_t indice)                                 // Funcion que prepara el nuevo estado comandado para la siguiente iteracion / Function defining the new aspect for the next iteration
{
  // Definicion de aspectos / Aspect definition
  // *** DEPENDIENTES DEL NUMERO DE FOCOS Y TIPO DE SEMAFOROS *** / *** DEPENDS ON THE SIGNAL TYPE AND NUMBER OF LIGHTS ***
  // Supone que Max_num_focos = 5 / Assumes Max_num_focos = 5


    switch (estado[indice])
    {
      case 0:                                 // --- Senal apagada / Signal off ---

        control[indice][0]       = 0;         // Verde apagado / Green off
        flag_parpadeo[indice][0] = false;
        control[indice][1]       = 0;         // Rojo apagado / Red off
        flag_parpadeo[indice][1] = false;
        control[indice][2]       = 0;         // Amarillo apagado / Yellow off
        flag_parpadeo[indice][2] = false;
        control[indice][3]       = 0;         // Blanco apagado / White off
        flag_parpadeo[indice][3] = false;
        control[indice][4]       = 0;         // Azul apagado / Blue off
        flag_parpadeo[indice][4] = false;

        focos_activos[indice] = 0;

        break;

      case 1:                                 // --- Via libre / Proceed ---

        control[indice][0]       = 5;         // Foco verde encendido / Green light on
        flag_parpadeo[indice][0] = false;

        focos_activos[indice] = 1;

        break;

      case 2:                                 // --- Via libre condicional / Proceed on condition ---

        control[indice][0]       = 5;         // Foco verde parpadeando / Green light blinking
        flag_parpadeo[indice][0] = true;

        focos_activos[indice] = 1;

        break;

      case 3:                                 // --- Anuncio de precaucion / Precaution warning ---

        control[indice][0]       = 5;         // Foco verde encendido / Green light on
        flag_parpadeo[indice][0] = false;
        control[indice][2]       = 5;         // Foco amarillo encendido / Yellow light on
        flag_parpadeo[indice][2] = false;

        focos_activos[indice] = 2;

        break;

      case 4:                                 // --- Preanuncio de parada / Distant stop warning ---

        control[indice][2]       = 5;         // Foco amarillo encendido / Yellow light on
        flag_parpadeo[indice][2] = false;
        control[indice][4]       = 5;         // Azul encendido (como cartelon) / Blue light on (working as screen)
        flag_parpadeo[indice][4] = false;

        focos_activos[indice] = 2;

        break;

      case 5:                                 // --- Anuncio de parada / Stop warning ---

        control[indice][2]       = 5;         // Foco amarillo encendido / Yellow light on
        flag_parpadeo[indice][2] = false;

        focos_activos[indice] = 1;

        break;

      case 6:                                 // --- Anuncio de parada inmediata / Immediate stop warning ---

        control[indice][2]       = 5;         // Foco amarillo parpadeando / Yellow light blinking
        flag_parpadeo[indice][2] = true;

        focos_activos[indice] = 1;

        break;

      // case 7:                                 // --- Parada diferida / Postponed stop ---
      //                                         // (No definido) / (To be defined)
      //   focos_activos[indice] = 1;
      //
      //   break;

      case 8:                                 // --- Parada / Stop ---

        control[indice][1]       = 5;         // Foco rojo encendido / Red light on
        flag_parpadeo[indice][1] = false;

        focos_activos[indice] = 1;

        break;

      case 9:                                 // --- Rebase autorizado (con parada) / Authorized pass (stopping on signal) ---

        control[indice][1]       = 5;         // Foco rojo encendido / Red light on
        flag_parpadeo[indice][1] = false;
        control[indice][3]       = 5;         // Foco blanco encendido / White light on
        flag_parpadeo[indice][3] = false;

        focos_activos[indice] = 2;

        break;

      case 10:                                // --- Rebase autorizado (sin parada) / Authorized pass (no stopping on signal) ---

        control[indice][1]       = 5;         // Foco rojo encendido / Red light on
        flag_parpadeo[indice][1] = false;
        control[indice][3]       = 5;         // Foco blanco parpadeando / White light blinking
        flag_parpadeo[indice][3] = true;

        focos_activos[indice] = 2;

        break;

      case 11:                                // --- Parada selectiva con luz azul fija / Selective stop --- (Sin ERTMS: parada / con ETCS nivel 2 en modo FS y con MA: continuar de acuerdo al DMI)

        control[indice][1]       = 5;         // Foco rojo encendido / Red light on
        flag_parpadeo[indice][1] = false;
        control[indice][4]       = 5;         // Foco azul encendido / Blue light on
        flag_parpadeo[indice][4] = false;

        focos_activos[indice] = 2;

        break;

      case 12:                                // --- Parada selectiva con luz azul parpadeando / Selective stop (with blinking blue light) --- (Sin ERTMS: parada / con ETCS nivel 1: continuar de acuerdo al DMI para obtener MA en baliza asociada, con ETCS nivel 2 en modo FS y con MA: continuar de acuerdo al DMI):

        control[indice][1]       = 5;         // Foco rojo encendido / Red light on
        flag_parpadeo[indice][1] = false;
        control[indice][4]       = 5;         // Foco azul parpadeando / Blue light blinking
        flag_parpadeo[indice][4] = true;

        focos_activos[indice] = 2;

        break;

      case 13:                                // --- Movimiento autorizado (con foco blanco) / Authorized movement (on white light signal) ---

        control[indice][3]       = 5;         // Foco blanco encendido / White light on
        flag_parpadeo[indice][3] = false;

        focos_activos[indice] = 1;

        break;

      case 14:                                // Movimiento autorizado (con foco azul) / Authorized movement (on blue light signal)

        control[indice][4]       = 5;         // Foco azul encendido / Blue light on
        flag_parpadeo[indice][4] = false;

        focos_activos[indice] = 1;

        break;

      case 15:                                // --- Indicacion de entrada a via directa / Switch entrance indication (straight track) ---

        control[indice][0]       = 5;         // Foco blanco superior encendido (se conecta a la salida verde) / Upper white light on (connected to green circuit)
        flag_parpadeo[indice][0] = false;
        control[indice][3]       = 5;         // Foco blanco inferior encendido (se conecta a la salida blanca) / Lower white light on (connected to white circuit)
        flag_parpadeo[indice][3] = false;

        focos_activos[indice] = 2;

        break;

      case 16:                                // --- Indicacion de entrada a via desviada / Switch entrance indication (diverging track)

        control[indice][3]       = 5;         // Foco blanco inferior encendido
        flag_parpadeo[indice][3] = false;
        control[indice][2]       = 5;         // Foco blanco lateral encendido (se conecta a la salida amarilla)
        flag_parpadeo[indice][2] = false;

        focos_activos[indice] = 2;

        break;

      // case 17:                                // --- Indicacion de salida / Departure indication ---
      //                                         // (No definido) / (To be defined)
      //
      //   focos_activos[indice] = 2;
      //
      //   break;

      case 18:                                // --- Parada en semaforos bajos de dos focos rojos / Stop (on dwarf signals with two red lights) ---

        control[indice][0]       = 5;         // Foco rojo superior encendido (se conecta a la salida verde) / Upper red light on (connected to green circuit)
        flag_parpadeo[indice][0] = false;
        control[indice][1]       = 5;         // Foco rojo inferior encendido (se conecta a la salida roja) / Lower red light on (connected to red circuit)
        flag_parpadeo[indice][1] = false;

        focos_activos[indice] = 2;

        break;

    }


  // Seleccion de limite de ciclo para equilibrar el brillo en anodo comun

    if (anodo_comun[indice] == true && focos_activos[indice] != 0)
    {
      factor_brillo[indice] = float(focos_activos[indice]) / 3.0;
      limite_ciclo[indice] = round(factor_brillo[indice] * 4095.0);
      paso_adaptado[indice] = round(factor_brillo[indice] * paso);
    }
    else
    {
      limite_ciclo[indice] = 4095;
      paso_adaptado[indice] = paso;
    }


}

uint8_t diccionario_pulsador_estado(uint8_t indice)               // Funcion que relaciona el pulsador de estado con el estado asignado / Function relating each push button with its assigned aspect

// *** DEPENDIENTE DE LOS ASPECTOS A CONTROLAR ELEGIDOS *** / *** DEPENDS ON THE CHOSEN ASPECTS TO BE CONTROLLED ***

{
  switch (indice)
  {
  case 0:
    return  8;  // Parada / Stop
    break;

  case 1:
    return  1;  // Via libre / Go
    break;

  case 2:
    return  2;  // Via libre condicional / Go on condition
    break;

  case 3:
    return  3;  // Anuncio de precaucion / Precaution warning
    break;

  case 4:
    return  5;  // Anuncio de parada / Stop warning
    break;

  case 5:
    return  6;  // Anuncio de parada inmediata / Immediate stop warning
    break;

  case 6:
    return  9;  // Rebase autorizado (con parada) / Authorized pass (stopping on signal)
    break;

  case 7:
    return 13;  // Movimiento autorizado / Authorized movement
    break;

  }
}


//---------------------------------------------------------------------------------------------------------------------------------------------------
// NOTA / NOTE
//---------------------------------------------------------------------------------------------------------------------------------------------------

// Lista de identificacion de aspectos / Aspect ID list
// Cada aspecto se representa por un numero entero / Each aspect will correlate with a number in this table

// ------------ Semaforos y senales RENFE/ADIF / Spanish RENFE/ADIF signals ------------

// 0  --> Semaforo apagado - Todos los focos apagados
// 1  --> Via libre - Luz verde fija
// 2  --> Via libre condicional - Luz verde parpadeando
// 3  --> Anuncio de precaucion - Luces verde y amarilla fijas
// 4  --> Preanuncio de parada - Luz amarilla fija y cartelon iluminado
// 5  --> Anuncio de parada - Luz amarilla fija
// 6  --> Anuncio de parada inmediata - Luz amarilla parpadeando
// 7  --> Parada diferida - Pantalla(s) visible(s)
// 8  --> Parada - Luz roja fija
// 9  --> Rebase autorizado (con parada) - Luces roja y blanca fijas
// 10 --> Rebase autorizado (sin parada) - Luz roja fija y blanca parpadeando
// 11 --> Parada selectiva con foco azul fijo - Luces roja y azul fijas (sin ERTMS: parada / con ETCS nivel 2 en modo FS y con MA: continuar de acuerdo al DMI)
// 12 --> Parada selectiva con foco azul parpadeando - Luz roja fija y azul parpadeando (sin ERTMS: parada / con ETCS nivel 1: continuar de acuerdo al DMI para obtener MA en baliza asociada, con ETCS nivel 2 en modo FS y con MA: continuar de acuerdo al DMI)
// 13 --> Movimiento autorizado (con foco blanco) - Luz blanca fija
// 14 --> Movimiento autorizado (con foco azul) - Luz azul fija
// 15 --> Indicacion de entrada (a via directa) - Luces blancas verticales
// 16 --> Indicacion de entrada (a via desviada) - Luces blancas horizontales
// 17 --> Indicacion de salida - Luz blanca vertical
// 18 --> Parada en semaforos bajos de dos focos rojos - Dos luces rojas fijas