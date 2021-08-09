// ======================================================================================================================
// DECODIFICADOR DE ACCESORIOS DCC PARA ARDUINO / DCC ACCESSORY DECODER FOR ARDUINO
// ======================================================================================================================
// v1.0 - Dario Calvo Sanchez, 2021 - Web: elgajoelegante.com - Twitter: @DarioCalvoS
// Decoder basado en libreria NmraDcc.h (https://github.com/mrrwa/NmraDcc) / Decoder based on library NmraDcc.h (https://github.com/mrrwa/NmraDcc)


// Compatible con los siguientes protocolos: / Compatible with the following protocols:
// - Turnout packet (Output Addressing Mode)
// - Turnout packet (Board Addressing Mode)
// - Signal Aspect Packet (Extended Accessory Decoder Control Packet Format)



// LISTA DE ASPECTOS DE LOS SEMAFOROS / SIGNAL ASPECT LIST
//
// ------------ Semaforos y senales RENFE/ADIF ------------
//
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
// 11 --> Parada selectiva con luz azul fija - Luces roja y azul fijas (sin ERTMS: parada / con ETCS nivel 2 en modo FS y con MA: continuar de acuerdo al DMI)
// 12 --> Parada selectiva con luz azul parpadeando - Luz roja fija y azul parpadeando (sin ERTMS: parada / con ETCS nivel 1: continuar de acuerdo al DMI para obtener MA en baliza asociada, con ETCS nivel 2 en modo FS y con MA: continuar de acuerdo al DMI)
// 13 --> Movimiento autorizado - Luz blanca fija
// 14 --> Movimiento autorizado (con foco azul) - Luz azul fija
// 15 --> Indicacion de entrada (a via directa) - Luces blancas verticales
// 16 --> Indicacion de entrada (a via desviada) - Luces blancas horizontales
// 17 --> Indicacion de salida - Luz blanca vertical
// 18 --> Parada en semaforos bajos de dos focos rojos - Dos luces rojas fijas


// VER TIPOS DE SEMAFORO DEFINIDOS EN LA FUNCION "diccionario_tipos" / SEE SIGNAL TYPES DEFINED IN FUNCTION "diccionario_tipos"



// --------------------------------------------------------------------------------------------------------------------------------------------
// LIBRERIAS / LIBRARIES
// --------------------------------------------------------------------------------------------------------------------------------------------

#include <NmraDcc.h>



// --------------------------------------------------------------------------------------------------------------------------------------------
// VARIABLES
// --------------------------------------------------------------------------------------------------------------------------------------------

// Variables para los semaforos / Signal variables

const uint8_t Numero_estados = 19;              // Numero total de aspectos definidos (ver lista arriba) / Max. number of possible defined aspects (see list above)
const uint8_t Numero_semaforos = 12;            // Numero total de semaforos instalados / Total number of signals

const uint8_t Numero_aspectos_DCC = 32;         // Maximo numero de aspectos "Signal Aspect" en el estandar DCC (32) / Max. number of aspects in "Signal aspect" DCC standard (32)

struct propiedades_semaforos                    // Estructura de definicion de los semaforos / Structure containing signal definitions
{
    uint8_t direccion_DCC;
    uint8_t tipo;
    uint8_t aspecto[Numero_aspectos_DCC];

    uint8_t estado_inicial;
    bool estados_posibles[Numero_estados];
    bool estados_actuales[Numero_estados];

};

struct comando_semaforos                        // Estructura para enviar a la placa de control / Structure to be sent to the control board
{
   uint8_t estado;
};

struct propiedades_semaforos semaforo[Numero_semaforos];
struct comando_semaforos comando[Numero_semaforos];


// Variables para la libreria y el control DCC / Variables for DCC library & control

NmraDcc  Dcc;
DCC_MSG  Packet;

const int DccAckPin = 5;                        // Pin para devolver un acuse de recibo a la central / Pin to acknowledge to command station

struct CVPair
{
  uint16_t  CV;
  uint8_t   Value;
};

CVPair FactoryDefaultCVs [] =
{
  {CV_ACCESSORY_DECODER_ADDRESS_LSB, 1},
  {CV_ACCESSORY_DECODER_ADDRESS_MSB, 0},
};

uint8_t FactoryDefaultCVIndex = 0;



// --------------------------------------------------------------------------------------------------------------------------------------------
// CONDICIONES INICIALES / INITIAL SETUP
// --------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{

  // Ajustes iniciales de los puertos serie / Serial ports setup

        Serial.begin(9600);                     // Puerto serie principal en Arduino / Arduino main serial port
        // Serial1.begin(9600);                    // Puerto adicional 1 en Arduino Mega / Arduino Mega additional port 1
        // Serial2.begin(9600);                    // Puerto adicional 2 en Arduino Mega / Arduino Mega additional port 2
        // Serial3.begin(9600);                    // Puerto adicional 3 en Arduino Mega / Arduino Mega additional port 3


  // Ajustes iniciales DCC / DCC initial setup

        // Pin de acuse de recibo / Acknowledge pin

            pinMode(DccAckPin, OUTPUT);

        // Ajuste de la interrupcion / Interrupt settings
        // Interrupcion (0), pin (2), configuracion pull-up (1 para resistencia interna) / Interrupt (0), pin (2), pull-up setting (1 for using internal resistor)

            Dcc.pin(0, 2, 1);

        // Inicio de la funcion DCC principal y su tipo (comentar el no deseado) / Start of main DCC function and type selection (comment the one discarded)

            // Dcc.init( MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER, 0 );                             // Modo Board Addressing (direccion y subdireccion) / Board Addressing mode (address & sub-address)
            Dcc.init( MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER | CV29_OUTPUT_ADDRESS_MODE, 0 );  // Modo Output Addressing (direccion unica) / Output Addressing mode (single address)


  // Definicion de los semaforos (comentar o anadir segun se requiera) / Signals definition (comment or add as required)

    // Semaforo 0: / Signal 0:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.15 (RENFE de salida con 4 focos, tipo 4)

        semaforo[0].direccion_DCC = 5;          // Direccion DCC / DCC address
        semaforo[0].tipo = 3;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[0].estado_inicial = 13;        // Aspecto inicial / Initial aspect

    // Semaforo 1: / Signal 1:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[1].direccion_DCC = 7;          // Direccion DCC / DCC address
        semaforo[1].tipo = 2;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[1].estado_inicial = 2;         // Aspecto inicial / Initial aspect

    // Semaforo 2: / Signal 2:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[2].direccion_DCC = 9;          // Direccion DCC / DCC address
        semaforo[2].tipo = 2;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[2].estado_inicial = 2;         // Aspecto inicial / Initial aspect

    // Semaforo 3: / Signal 3:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[3].direccion_DCC = 11;         // Direccion DCC / DCC address
        semaforo[3].tipo = 2;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[3].estado_inicial = 2;         // Aspecto inicial / Initial aspect

    // Semaforo 4: / Signal 4:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[4].direccion_DCC = 13;         // Direccion DCC / DCC address
        semaforo[4].tipo = 2;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[4].estado_inicial = 2;         // Aspecto inicial / Initial aspect

    // Semaforo 5: / Signal 5:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[5].direccion_DCC = 15;         // Direccion DCC / DCC address
        semaforo[5].tipo = 2;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[5].estado_inicial = 2;         // Aspecto inicial / Initial aspect

    // Semaforo 6: / Signal 6:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[6].direccion_DCC = 17;         // Direccion DCC / DCC address
        semaforo[6].tipo = 2;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[6].estado_inicial = 2;         // Aspecto inicial / Initial aspect

    // Semaforo 7: / Signal 7:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[7].direccion_DCC = 19;         // Direccion DCC / DCC address
        semaforo[7].tipo = 2;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[7].estado_inicial = 2;         // Aspecto inicial / Initial aspect

    // Semaforo 8: / Signal 8:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[8].direccion_DCC = 21;         // Direccion DCC / DCC address
        semaforo[8].tipo = 2;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[8].estado_inicial = 2;         // Aspecto inicial / Initial aspect

    // Semaforo 9: / Signal 9:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[9].direccion_DCC = 23;         // Direccion DCC / DCC address
        semaforo[9].tipo = 2;                   // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[9].estado_inicial = 2;         // Aspecto inicial / Initial aspect

    // Semaforo 10: / Signal 10:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[10].direccion_DCC = 25;        // Direccion DCC / DCC address
        semaforo[10].tipo = 2;                  // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[10].estado_inicial = 2;        // Aspecto inicial / Initial aspect

    // Semaforo 11: / Signal 11:
    // Referencia / Reference: Ejemplo / Example Mafen 4131.11 (RENFE principal con 3 focos, tipo 2)

        semaforo[11].direccion_DCC = 27;        // Direccion DCC / DCC address
        semaforo[11].tipo = 2;                  // Tipo de semaforo (ver funcion "diccionario_tipos") / Signal type (see function "diccionario_tipos")
        semaforo[11].estado_inicial = 2;        // Aspecto inicial / Initial aspect


  // Ajustes iniciales de los semaforos / Signals initial setup

    for (uint8_t i = 0; i < Numero_semaforos; i++)
    {

        for (uint8_t j = 0; j < Numero_estados; j++)
        {
            semaforo[i].estados_posibles[j] = false;
            semaforo[i].estados_actuales[j] = false;
        }

        for (uint8_t k = 0; k < Numero_aspectos_DCC; k++)
        {
            semaforo[i].aspecto[k] = 255;
        }

        diccionario_tipos(i, semaforo[i].tipo, true);

        comando[i].estado = semaforo[i].estado_inicial;

    }

    envia_estructura((byte*)&comando, sizeof(comando), 0);

}



// --------------------------------------------------------------------------------------------------------------------------------------------
// BUCLE PRINCIPAL / MAIN LOOP
// --------------------------------------------------------------------------------------------------------------------------------------------

void loop()
{

    // Llamada regular a la funcion DCC / Regular call to function DCC

    Dcc.process();

    if( FactoryDefaultCVIndex && Dcc.isSetCVReady())        // En caso de reset del deco / In case of decoder reset
    {
        FactoryDefaultCVIndex--;
        Dcc.setCV( FactoryDefaultCVs[FactoryDefaultCVIndex].CV, FactoryDefaultCVs[FactoryDefaultCVIndex].Value);
    }

}



// --------------------------------------------------------------------------------------------------------------------------------------------
// FUNCIONES / FUNCTIONS
// --------------------------------------------------------------------------------------------------------------------------------------------

void notifyCVResetFactoryDefault()                                                                                  // Funcion para el reset del deco virtual / Reset function
{
  FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs)/sizeof(CVPair);
};


void notifyCVAck(void)                                                                                              // Funcion de acuse de recibo / Acknowledge function to command station
{
  digitalWrite( DccAckPin, HIGH );
  delay( 6 );
  digitalWrite( DccAckPin, LOW );
}


void notifyDccAccTurnoutBoard( uint16_t BoardAddr, uint8_t OutputPair, uint8_t Direction, uint8_t OutputPower )     // Funcion para ejecutar al recibir un paquete DCC del tipo "Turnout" en modo "Board Addressing Mode" / Function to be run when receiving a "Turnout - Board Addressing mode" DCC packet
{
  // Devuelve 4 valores: / Returns 4 values:
  // BoardAddr, OutputPair, Direction, OutputPower

  // Introducir instrucciones deseadas a continuacion en caso de recepcion: / Add here instructions to be run in case of reception of this packet type:

  // Salida a monitor serie: / Serial monitor output:
    //   Serial.print("notifyDccAccTurnoutBoard: ");
    //   Serial.print(BoardAddr,DEC);
    //   Serial.print(',');
    //   Serial.print(OutputPair,DEC);
    //   Serial.print(',');
    //   Serial.print(Direction,DEC);
    //   Serial.print(',');
    //   Serial.println(OutputPower, HEX);

}


void notifyDccAccTurnoutOutput( uint16_t Addr, uint8_t Direction, uint8_t OutputPower )                             // Funcion para ejecutar al recibir un paquete DCC del tipo "Turnout" en modo "Output Addressing Mode" / Function to be run when receiving a "Turnout - Output Addressing mode" DCC packet
{
  // Devuelve 3 valores: / Returns 3 values:
  // Addr, Direction, OutputPower

  // Introducir instrucciones deseadas a continuacion en caso de recepcion: / Add here instructions to be run in case of reception of this packet type:

  // Salida a monitor serie: / Serial monitor output:
    //   Serial.print("notifyDccAccTurnoutOutput: ");
    //   Serial.print(Addr,DEC);
    //   Serial.print(',');
    //   Serial.print(Direction,DEC);
    //   Serial.print(',');
    //   Serial.println(OutputPower, HEX);

}


void notifyDccSigOutputState( uint16_t Addr, uint8_t State)                                                         // Funcion para ejecutar al recibir un paquete DCC del tipo "Signal Aspect" / Function to be run when receiving a "Signal aspect" DCC packet
{
  // Los paquetes DCC del tipo "Signal Aspect Packet" son independientes de que la configuracion en Dcc.init() sea "Output" o "Board" / "Signal aspect" packets are independent on Dcc.init() configuration (Board or Output addressing mode)

  // Devuelve 3 valores: / Returns 3 values:
  // Addr, State

  // Introducir instrucciones deseadas a continuacion en caso de recepcion: / Add here instructions to be run in case of reception of this packet type:

    // Salida a monitor serie: / Serial monitor output:
        //   Serial.print("notifyDccSigOutputState: ");
        //   Serial.print(Addr,DEC);
        //   Serial.print(',');
        //   Serial.println(State, HEX);

    for (uint8_t i = 0; i < Numero_semaforos; i++)
    {
        if (Addr == semaforo[i].direccion_DCC)
        {
            if (comando[i].estado != semaforo[i].aspecto[State])
            {
                comando[i].estado = semaforo[i].aspecto[State];
                envia_estructura((byte*)&comando, sizeof(comando), 0);  // Envia el array con el comando de los semaforos / Sends command array to control board
            }
            break;
        }

    }

}


void envia_estructura(byte *puntero_estructura, int long_estructura, uint8_t puerto)                                // Funcion para enviar una estructura por los puertos serie / Function to send an structure through serial ports
{
    if (puerto == 0)                                            // Puerto principal (por defecto) / Default serial port
    {
        Serial.write(puntero_estructura, long_estructura);
    }

    // if (puerto == 1)                                            // Puerto adicional 1 en Arduino Mega / Arduino Mega additional port 1
    // {
    //     Serial1.write(puntero_estructura, long_estructura);
    // }

    // if (puerto == 2)                                            // Puerto adicional 2 en Arduino Mega / Arduino Mega additional port 2
    // {
    //     Serial2.write(puntero_estructura, long_estructura);
    // }

    // if (puerto == 3)                                            // Puerto adicional 3 en Arduino Mega / Arduino Mega additional port 3
    // {
    //     Serial3.write(puntero_estructura, long_estructura);
    // }

}


void diccionario_tipos(uint8_t id_semaforo, uint8_t tipo, bool procesa_estados)                                     // Funcion para relacionar el aspecto emitido por la central DCC con el aspecto interno de cada semaforo / Function to relate aspects commanded by DCC command station with each signal's own aspect definition
{
    // Opcionalmente, asigna los estados posibles y actuales en el bucle inicial si "procesa_estados" es "true" / Optionally, assigns possible & actual aspects on the initial setup loop if "procesa_estados" is "true"

    uint8_t aspectos;

    switch (tipo)
    {

        case 1:

            // Semaforo principal o bajo RENFE/ADIF con 2 focos: verde, rojo
            // Ejemplo: Mafen 4131.02, 4131.04, 4131.12 o 4141.01
            // 4 aspectos posibles

            aspectos = 4;

            semaforo[id_semaforo].aspecto[0] =  8;  // Parada
            semaforo[id_semaforo].aspecto[1] =  1;  // Via libre
            semaforo[id_semaforo].aspecto[2] =  2;  // Via libre condicional
            semaforo[id_semaforo].aspecto[3] =  0;  // Apagado

            break;

        case 2:

            // Semaforo principal o bajo RENFE/ADIF con 3 focos: verde, rojo, amarillo
            // Ejemplo: Mafen 4131.01, 4131.11, 4141.04 o 4141.05
            // 7 aspectos posibles

            aspectos = 7;

            semaforo[id_semaforo].aspecto[0] =  8;  // Parada
            semaforo[id_semaforo].aspecto[1] =  1;  // Via libre
            semaforo[id_semaforo].aspecto[2] =  2;  // Via libre condicional
            semaforo[id_semaforo].aspecto[3] =  3;  // Anuncio de precaucion
            semaforo[id_semaforo].aspecto[4] =  5;  // Anuncio de parada
            semaforo[id_semaforo].aspecto[5] =  6;  // Anuncio de parada inmediata
            semaforo[id_semaforo].aspecto[6] =  0;  // Apagado

            break;

        case 3:

            // Semaforo de salida o bajo RENFE/ADIF con 4 focos: verde, rojo, amarillo, blanco
            // Ejemplo: Mafen 4131.03, 4131.15, 4141.06 o 4141.07
            // 10 aspectos posibles

            aspectos = 10;

            semaforo[id_semaforo].aspecto[0] =  8;  // Parada
            semaforo[id_semaforo].aspecto[1] =  1;  // Via libre
            semaforo[id_semaforo].aspecto[2] =  2;  // Via libre condicional
            semaforo[id_semaforo].aspecto[3] =  3;  // Anuncio de precaucion
            semaforo[id_semaforo].aspecto[4] =  5;  // Anuncio de parada
            semaforo[id_semaforo].aspecto[5] =  6;  // Anuncio de parada inmediata
            semaforo[id_semaforo].aspecto[6] =  9;  // Rebase autorizado (con parada)
            semaforo[id_semaforo].aspecto[7] = 10;  // Rebase autorizado (sin parada)
            semaforo[id_semaforo].aspecto[8] = 13;  // Movimiento autorizado
            semaforo[id_semaforo].aspecto[9] =  0;  // Apagado

            break;

        case 4:

            // Semaforo de salida RENFE/ADIF con 4 focos: verde, rojo, amarillo, azul
            // Ejemplo: Mafen 4131.16
            // 10 aspectos posibles

            aspectos = 10;

            semaforo[id_semaforo].aspecto[0] =  8;  // Parada
            semaforo[id_semaforo].aspecto[1] =  1;  // Via libre
            semaforo[id_semaforo].aspecto[2] =  2;  // Via libre condicional
            semaforo[id_semaforo].aspecto[3] =  3;  // Anuncio de precaucion
            semaforo[id_semaforo].aspecto[4] =  5;  // Anuncio de parada
            semaforo[id_semaforo].aspecto[5] =  6;  // Anuncio de parada inmediata
            semaforo[id_semaforo].aspecto[6] = 11;  // Parada selectiva con luz azul fija
            semaforo[id_semaforo].aspecto[7] = 12;  // Parada selectiva con luz azul parpadeando
            semaforo[id_semaforo].aspecto[8] = 14;  // Movimiento autorizado (con foco azul)
            semaforo[id_semaforo].aspecto[8] =  0;  // Apagado

            break;

        case 5:

            // Semaforo de avanzada RENFE/ADIF con 2 focos: verde, amarillo
            // Ejemplo: Mafen 4131.05 o 4131.13
            // 6 aspectos posibles

            aspectos = 6;

            semaforo[id_semaforo].aspecto[0] =  6;  // Anuncio de parada inmediata (segun el estandar DCC el comando de aspecto 0 siempre es parada absoluta, pero no es aplicable en este semaforo, asi que se asimila al siguiente estado mas restrictivo)
            semaforo[id_semaforo].aspecto[1] =  1;  // Via libre
            semaforo[id_semaforo].aspecto[2] =  2;  // Via libre condicional
            semaforo[id_semaforo].aspecto[3] =  3;  // Anuncio de precaucion
            semaforo[id_semaforo].aspecto[4] =  5;  // Anuncio de parada
            semaforo[id_semaforo].aspecto[5] =  0;  // Apagado

            break;

        case 6:

            // Semaforo de entrada RENFE/ADIF con 2 focos: rojo, amarillo
            // Ejemplo: Mafen 4131.14
            // 4 aspectos posibles

            aspectos = 4;

            semaforo[id_semaforo].aspecto[0] =  8;  // Parada
            semaforo[id_semaforo].aspecto[1] =  5;  // Anuncio de parada
            semaforo[id_semaforo].aspecto[2] =  6;  // Anuncio de parada inmediata
            semaforo[id_semaforo].aspecto[3] =  0;  // Apagado

            break;

        case 7:

            // Semaforo de salida RENFE/ADIF con 3 focos: verde, rojo, blanco
            // Ejemplo: Mafen 4131.17
            // 7 aspectos posibles

            aspectos = 7;

            semaforo[id_semaforo].aspecto[0] =  8;  // Parada
            semaforo[id_semaforo].aspecto[1] =  1;  // Via libre
            semaforo[id_semaforo].aspecto[2] =  2;  // Via libre condicional
            semaforo[id_semaforo].aspecto[3] =  9;  // Rebase autorizado (con parada)
            semaforo[id_semaforo].aspecto[4] = 10;  // Rebase autorizado (sin parada)
            semaforo[id_semaforo].aspecto[5] = 13;  // Movimiento autorizado
            semaforo[id_semaforo].aspecto[6] =  0;  // Apagado

            break;

        case 8:

            // Semaforo bajo RENFE/ADIF con 2 focos: rojo, blanco
            // Ejemplo: Mafen 4141.02
            // 5 aspectos posibles

            aspectos = 5;

            semaforo[id_semaforo].aspecto[0] =  8;  // Parada
            semaforo[id_semaforo].aspecto[1] =  9;  // Rebase autorizado (con parada)
            semaforo[id_semaforo].aspecto[2] = 10;  // Rebase autorizado (sin parada)
            semaforo[id_semaforo].aspecto[3] = 13;  // Movimiento autorizado
            semaforo[id_semaforo].aspecto[4] =  0;  // Apagado

            break;

        case 9:

            // Semaforo bajo RENFE/ADIF con 2 focos rojos
            // Ejemplo: Mafen 4141.03
            // 2 aspectos posibles

            aspectos = 2;

            semaforo[id_semaforo].aspecto[0] = 18;  // Parada
            semaforo[id_semaforo].aspecto[1] =  0;  // Apagado

            break;

        case 10:

            // Semaforo bajo indicador de entrada RENFE/ADIF con 4 focos: 1 rojo, 3 blancos
            // Ejemplo: Mafen 4141.08 o 4141.09
            // 7 aspectos posibles

            aspectos = 7;

            semaforo[id_semaforo].aspecto[0] =  8;  // Parada
            semaforo[id_semaforo].aspecto[1] =  9;  // Rebase autorizado (con parada)
            semaforo[id_semaforo].aspecto[2] = 10;  // Rebase autorizado (sin parada)
            semaforo[id_semaforo].aspecto[3] = 13;  // Movimiento autorizado
            semaforo[id_semaforo].aspecto[4] = 15;  // Indicacion de entrada a via directa
            semaforo[id_semaforo].aspecto[5] = 16;  // Indicacion de entrada a via desviada
            semaforo[id_semaforo].aspecto[6] =  0;  // Apagado

            break;

    }


    if (procesa_estados == true)
    {
        for (uint8_t m = 0; m < aspectos; m++)
        {
            semaforo[id_semaforo].estados_posibles[semaforo[id_semaforo].aspecto[m]] = true;
            semaforo[id_semaforo].estados_actuales[semaforo[id_semaforo].aspecto[m]] = true;
        }
    }


}
