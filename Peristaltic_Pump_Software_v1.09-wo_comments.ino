char vers[] = "1.0.9";

// include the library code:
//#include <LiquidCrystal.h> //https://www.arduino.cc/en/Reference/LiquidCrystal -> LCD control
#include <ClickEncoder.h> //https://github.com/0xPIT/encoder/blob/master/README.md -> Encoder processing (timer based)
#include <TimerOne.h> //required for ClickEncoder.h 
#include <EEPROM.h> //write and read EEPROM (to save and load settings)
#include <LiquidCrystal_I2C.h>  //https://github.com/johnrickman/LiquidCrystal_I2C
#include <Wire.h>
//LCD -----------------------------------------------------------------------------------

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define LCD_COLUMNS 16
#define LCD_ROWS 2
// initialize the library with the numbers of the interface pins
//LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_EN, LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);

//ENCODER --------------------------------------------------------------------------------
#define ENCODER_PIN_BUTTON 4
#define ENCODER_PIN_A 3
#define ENCODER_PIN_B 2
ClickEncoder *encoder;
int16_t last, value;

//STEP MOTOR -----------------------------------------------------------------------------
#define MOTOR_ENABLE_PIN 9
#define MOTOR_STEP_PIN 7
#define MOTOR_DIR_PIN 6
#define STEP_MODE 4 // (1: Full Step, 2: Half Step, 4: Quarter Step, ...)
#define STEPS_PER_FULL_ROT 200 // @ full steps
long delay_us;
long steps;
long step_counter = 0;
unsigned long pausa;


//LED indicador----------------------
const int LEDdosifica = 10;
const int LEDbombea = 11;
const int LEDcalibra = 12;



//SERIAL COMMUNICATION ---------------------------------------------------------------------
#define BAUD 9600

//STATE ------------------------------------------------------------------------------------
boolean in_menu = 0;
volatile boolean in_action = 0;
boolean menu_entered = 0;
boolean menu_left = 0;

//GENERAL -----------------------------------------------------------------------------------
#define MICROSEC_PER_SEC 1000000

//MENU ---------------------------------------------------------------------------------------
#define MAX_NUM_OF_OPTIONS 4
#define NUM_OF_MENU_ITEMS 11
#define VALUE_MAX_DIGITS 4
#define VALUE_MAX_RPM 200

int menu_number_1 = 0;
int menu_number_2 = 1;
boolean val_change = 0;
double value_dbl;
char value_str[VALUE_MAX_DIGITS + 1];


enum menu_type {
  VALUE,
  OPTION,
  ACTION
};

typedef struct
{
  const char* name_;
  menu_type type; //0: value type, 1:option type, 2:action type
  int value;
  int decimals;
  int lim;
  const char* options[4];
  const char* suffix;
} menu_item;
int menu_items_limit = 11 - 1;
menu_item menu[11];

int ciclo = 1;
bool noGota = true;
bool inwhile = false;
bool accel = true;

//███ SETUP ████████████████████████████████████████████████████████████████████████████████████████████████████
void setup() {
  // Inicializar el LCD
  Wire.begin(); //inicia biblioteca wire, asigna SDA a pin0 (D3) y SCL a pin 5 (D1);
  lcd.init();
  lcd.backlight();
  pinMode(MOTOR_STEP_PIN, OUTPUT);
  pinMode(MOTOR_DIR_PIN, OUTPUT);
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  digitalWrite(MOTOR_DIR_PIN, LOW);
  digitalWrite(MOTOR_STEP_PIN, LOW);
  digitalWrite(MOTOR_ENABLE_PIN, HIGH);

  pinMode(LEDdosifica, OUTPUT);
  pinMode(LEDbombea, OUTPUT);
  pinMode(LEDcalibra, OUTPUT);
  digitalWrite(LEDdosifica, LOW);
  digitalWrite(LEDbombea, LOW);
  digitalWrite(LEDcalibra, LOW);


  menu[0].name_ = "Start ";
  menu[0].type = ACTION;
  menu[0].value = 0;
  menu[0].lim = 0;
  menu[0].suffix = " RUN";

  menu[1].name_ = "Tiempo: ";
  menu[1].type = VALUE;
  menu[1].value = 50;
  menu[1].decimals = 1;
  menu[1].lim = 600;
  menu[1].suffix = "s";

  menu[2].name_ = "Vel: ";
  menu[2].type = VALUE;
  menu[2].value = 60;
  menu[2].decimals = 0;
  menu[2].lim = VALUE_MAX_RPM;
  menu[2].suffix = "RPM";

  menu[3].name_ = "Ciclos: ";
  menu[3].type = VALUE;
  menu[3].value = 2;
  menu[3].decimals = 0;
  menu[3].lim = 200;
  menu[3].suffix = "";

  menu[4].name_ = "Pausa: ";
  menu[4].type = VALUE;
  menu[4].value = 15;
  menu[4].decimals = 1;
  menu[4].lim = 300;
  menu[4].suffix = "s";

  menu[5].name_ = "Modo: ";
  menu[5].type = OPTION;
  menu[5].value = 0;
  menu[5].lim = 3 - 1;
  menu[5].options[0] = "Dosifica";
  menu[5].options[1] = "Bombea";
  menu[5].options[2] = "Calibra";

  menu[6].name_ = "PGM: ";
  menu[6].type = OPTION;
  menu[6].value = 0;
  menu[6].lim = 2 - 1;
  menu[6].options[0] = "Moscas";
  menu[6].options[1] = "Gusanos";

  menu[7].name_ = "Sentido: ";
  menu[7].type = OPTION;
  menu[7].value = 0;
  menu[7].lim = 2 - 1;
  menu[7].options[0] = "H";
  menu[7].options[1] = "AH";

  menu[8].name_ = "M ";
  menu[8].type = VALUE;
  menu[8].value = 4000;
  menu[8].decimals = 0;
  menu[8].lim = 1000000;
  menu[8].suffix = " pasos";

  menu[9].name_ = "G ";
  menu[9].type = VALUE;
  menu[9].value = 4000;
  menu[9].decimals = 0;
  menu[9].lim = 1000000;
  menu[9].suffix = " pasos";

  menu[10].name_ = "Guardar Conf.";
  menu[10].type = ACTION;
  menu[10].value = 0;
  menu[10].lim = 0;
  menu[10].suffix = "OK!";

  for (int i = 0; i <= menu_items_limit; i++) {
    menu[i].value = eepromReadInt(i * 2);
  }

  encoder = new ClickEncoder(ENCODER_PIN_B, ENCODER_PIN_A, ENCODER_PIN_BUTTON, 4); //(Encoder A, Encoder B, PushButton)
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  last = 0;

  //  Serial.begin(BAUD);

  // Print a message to the LCD.
  lcd.print("BR - ");
  //  lcd.setCursor(0, 1);
  lcd.print(vers);
  //  lcd.print(__DATE__);
  digitalWrite(LEDdosifica, HIGH);
  digitalWrite(LEDbombea, HIGH);
  digitalWrite(LEDcalibra, HIGH);
  delay(1000);
  digitalWrite(LEDdosifica, LOW);
  digitalWrite(LEDbombea, LOW);
  digitalWrite(LEDcalibra, LOW);


  //  ajusta las RPM en función del tiempo establecido en menu1 y la opcion de programa
  //menu[2].value = timeToRPM();
  menu[1].value = rpmToTIME();
  led();
  update_lcd();

  if (menu[7].value == 0) {
    digitalWrite(MOTOR_DIR_PIN, LOW);
  } else {
    digitalWrite(MOTOR_DIR_PIN, HIGH);
  }
  steps = steps_calc(menu[6].value, menu[8].value, menu[9].value);
  delay_us = delay_us_calc(menu[2].value);

}

//███ LOOP ████████████████████████████████████████████████████████████████████████████████████████████████████

void loop() {

  // BUTTON HANDLING ////////////////////////////////////////////////////////////////////////////////

  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Clicked:
        if (menu[menu_number_1].type == VALUE || menu[menu_number_1].type == OPTION) { // if value or option type
          in_menu = !in_menu;
        }
        if (menu[menu_number_1].type == ACTION) { // if action type
          in_action = !in_action;
          step_counter = 0;
          ciclo = 1;
        }

        if (in_action == true || in_menu == true) { //menu entered
          menu_entered = true;
        }

        if (in_action == false && in_menu == false) { //menu left
          menu_left = true;
        }
        if (in_action == false) {
          digitalWrite(MOTOR_ENABLE_PIN, HIGH);
          noGota = true;
          accel = true;
        }
        break;

      case ClickEncoder::DoubleClicked:
        if (menu[menu_number_1].type == VALUE) {
          menu[menu_number_1].value = menu[menu_number_1].value + menu[menu_number_1].lim / 10;
          if (menu[menu_number_1].value > menu[menu_number_1].lim)(menu[menu_number_1].value = 0);
          val_change = true;
          menu_left = true;
        }
        break;
      case ClickEncoder::Held:  //va a bombear. la idea es que se llene la manguera
        digitalWrite(MOTOR_ENABLE_PIN, LOW);
        if (accel == true) {
          acelerar(delay_us);
          accel = false;
        } else {
          pump(delay_us);
        }
        break;

      case ClickEncoder::Released:
        digitalWrite(MOTOR_ENABLE_PIN, HIGH);
        accel = true;
        break;
    }
  }


  /// SETUP ////////////////////////////////////////////////////////////////////////////////

  if (menu_entered) {
    lcd.blink();
    if (menu[menu_number_1].type == ACTION) {
      if (menu_number_1 == 0 && menu[5].value == 0) {
        lcd.setCursor((strlen(menu[menu_number_1].name_) + 2), 0);
        lcd.print(ciclo);
        lcd.print("/");
        lcd.print(menu[3].value);
        lcd.print("     ");
      } else {
        lcd.setCursor((LCD_COLUMNS - strlen(menu[menu_number_1].suffix)), 0);
        lcd.print(menu[menu_number_1].suffix);
        lcd.setCursor(15, 0);
      }
    }
    if (menu[menu_number_1].type == VALUE) {
      encoder->setAccelerationEnabled(true);
    }
    menu_entered = false;
  }



  /// ACTIONS ////////////////////////////////////////////////////////////////////////////////

  if (in_action) {
    switch (menu_number_1) {
      case 0: //Start

        if (menu[5].value == 0) { //Dose
          if (dose(steps, delay_us, step_counter, menu[3].value)) {
            exit_action_menu();
          }
        } else if (menu[5].value == 1) {      //Pump
          digitalWrite(MOTOR_ENABLE_PIN, LOW);
          if (accel == true) {
            acelerar(delay_us);
            accel = false;
          } else {
            pump(delay_us);
          }

        } else if (menu[5].value == 2) {      //Cal.
          if (calibrarMoscasGusanos()) {
            exit_action_menu();
          }
        }
        break;

      case 10:
        for (int i = 0; i <= menu_items_limit; i++) {
          eepromWriteInt(i * 2, menu[i].value);
        }
        delay(700);
        menu_left = true;
        break;
    }

    /// MENU (no action) ////////////////////////////////////////////////////////////////////////////////

  } else if (!in_action) {
    digitalWrite(MOTOR_ENABLE_PIN, HIGH);
    if (val_change == true) {
      update_lcd();
      val_change == false;
    }

    value += encoder->getValue(); // encoder update

    if (!in_menu) { // no menu selected
      val_change = encoder_selection(menu_number_1, menu_number_2, menu_items_limit); //process value change. cambia menu

    } else if (in_menu) { // menu selected
      if (menu[menu_number_1].type == 0) {
        val_change = encoder_value_selection(menu[menu_number_1].value, menu[menu_number_1].lim);
      } else {
        val_change = encoder_selection(menu[menu_number_1].value, menu[menu_number_1].lim); //////////////////////////////
        ///////////////////////////////////////////////////////////////////////////hay dos funciones encoder selection. no habrá que cambiarles el nombre? NO--> se elige la función según el número de parámetros
      }
    }

  }

  /// CLOSE ////////////////////////////////////////////////////////////////////////////////

  if (menu_left) {
    lcd.noBlink();
    if (menu[menu_number_1].type == ACTION) {
      exit_action_menu();
    }
    if (menu[menu_number_1].type == VALUE) {
      encoder->setAccelerationEnabled(false);
    }

    if (menu_number_1 == 1) {  //tiempo
      menu[2].value = timeToRPM();
    }

    if (menu_number_1 == 2) {    //velocidad
      menu[1].value = rpmToTIME();
    }

    if (menu_number_1 == 5) {    //modo
      menu[1].value = rpmToTIME();
      led();
    }

    if (menu_number_1 == 6) {    //PGM
      menu[1].value = rpmToTIME();

    }
    if (menu_number_1 == 7) { //Change Direction
      if (menu[7].value == 0) {
        digitalWrite(MOTOR_DIR_PIN, LOW);
      } else {
        digitalWrite(MOTOR_DIR_PIN, HIGH);
      }
    }

    if (menu_number_1 == 8 || menu_number_1 == 9) {    //pasos
      menu[1].value = rpmToTIME();
    }

    steps = steps_calc(menu[6].value, menu[8].value, menu[9].value);
    delay_us = delay_us_calc(menu[2].value);
    update_lcd();
    menu_left = false;
  }

}

//███ FUNCTION DECLARATION █████████████████████████████████████████████████████████████████████████████████████████████████

//_____________________________________________________________________________________________

void timerIsr() {
  encoder->service();
}

//_____________________________________________________________________________________________

boolean dose(long _steps, int _delay_us, long & inc, int cycle) {
  if (ciclo <= cycle) {
    while (inc < 800) {    ////en 1.0.6 era 400, o sea media vuelta
      digitalWrite(MOTOR_ENABLE_PIN, LOW);
      double d = (-0.0025 * double(inc)) + 3;  ///cambié -0.005 por -0.0025 para que al llegar a 800 pasos llegue al valor de velocidad máxima(o delay minimo)
      int c = d * _delay_us;
      for (int i = 0; i < 4; i++) {
        digitalWrite(MOTOR_STEP_PIN, HIGH);
        delayMicroseconds(c);
        digitalWrite(MOTOR_STEP_PIN, LOW);
        delayMicroseconds(c);
      }
      inc = inc + 4;

    }

    while (inc <= _steps) {
      digitalWrite(MOTOR_STEP_PIN, HIGH);
      delayMicroseconds(_delay_us);
      digitalWrite(MOTOR_STEP_PIN, LOW);
      delayMicroseconds(_delay_us);
      inc++;

    }

    if (noGota) {
      lcd.setCursor(15, 0);
      lcd.print("P");
      delay(50);
      antiGoteo();
      pausa = millis();
      noGota = false;
    }

    if (millis() - pausa < menu[4].value * 100) {
      return false;
    } else {
      ciclo++;
      inc = 0;
      menu_entered = true;
      noGota = true;
      return false;
    }
  } else {
    inc = 0;
    ciclo = 1;
    return true;
  }
}


//__________________________________________________

boolean calibrarMoscasGusanos() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Presionar hasta");
  lcd.setCursor(0, 1);
  lcd.print("llenar el tubo");
  //ClickEncoder::Button b = encoder->getButton();

  while (digitalRead(ENCODER_PIN_BUTTON) == HIGH) {
    delay(100);
  }
  unsigned long pasos = 0;

  digitalWrite(MOTOR_ENABLE_PIN, LOW);
  while (digitalRead(ENCODER_PIN_BUTTON) == LOW) {
    digitalWrite(MOTOR_STEP_PIN, HIGH);
    delayMicroseconds(1500 / STEP_MODE); //2.0ms
    digitalWrite(MOTOR_STEP_PIN, LOW);
    delayMicroseconds(1500 / STEP_MODE);
    pasos++;
  }
  antiGoteo();
  if (menu[6].value == 0) { //programa moscas
    menu[8].value = pasos;
    menu[1].value = rpmToTIME();
  } else if (menu[6].value == 1) {  //programa gusanos
    menu[9].value = pasos;
    menu[1].value = rpmToTIME();
  }
  menu[5].value = 0;

  double revoluciones = double(pasos) / (STEPS_PER_FULL_ROT * STEP_MODE);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pasos: ");
  lcd.print(pasos);
  lcd.setCursor(0, 1);
  lcd.print("rot: ");
  lcd.print(revoluciones, 1);
  delay(3000);
  led();
  lcd.clear();
  update_lcd();
  menu_left = true;
  return true;

}

//_____________________________________________________________________________________________
void antiGoteo() {
  long a = millis();
  boolean direccion = digitalRead(MOTOR_DIR_PIN);
  digitalWrite(MOTOR_DIR_PIN, !direccion);   //cambia sentido de giro
  delay(150);  //pausa agregada en 1.0.7 beta
  for (int i = 0; i < 50; i++) { ///retrocede 15x2.8 grados=42 grados
    digitalWrite(MOTOR_STEP_PIN, HIGH);
    delayMicroseconds(500);
    digitalWrite(MOTOR_STEP_PIN, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(MOTOR_DIR_PIN, direccion);   //restablece el sentido de giro
  digitalWrite(MOTOR_ENABLE_PIN, HIGH);
}
//_____________________________________________________________________________________________

void acelerar(int _delay_us) {
  if (_delay_us < 73) {
    _delay_us = 73;
  }
  int pasos = 0;
  digitalWrite(MOTOR_ENABLE_PIN, LOW);
  while (pasos < 800) {    ////en 1.0.6 era 400, o sea media vuelta
    double d = (-0.0025 * double(pasos)) + 3;
    int c = d * _delay_us;
    for (int i = 0; i < 4; i++) {
      //digitalWrite(MOTOR_ENABLE_PIN, LOW);
      digitalWrite(MOTOR_STEP_PIN, HIGH);
      delayMicroseconds(c);
      digitalWrite(MOTOR_STEP_PIN, LOW);
      delayMicroseconds(c);
    }
    pasos = pasos + 4;
  }
}

//_____________________________________________________________________________________________

void pump(int _delay_us) {
  if (_delay_us < 73) {
    _delay_us = 73;
  }
  digitalWrite(MOTOR_STEP_PIN, HIGH);
  delayMicroseconds(_delay_us);
  digitalWrite(MOTOR_STEP_PIN, LOW);
  delayMicroseconds(_delay_us);
}
//_____________________________________________________________________________________________

void exit_action_menu() {
  in_action = false;

  digitalWrite(MOTOR_ENABLE_PIN, HIGH);
  update_lcd();
  lcd.noBlink();
}
//_____________________________________________________________________________________________

long steps_calc(bool pgm, int pasos_moscas, int pasos_gusanos) {
  if (pgm == 0) {   //pgm moscas
    return pasos_moscas;
  } else if (pgm == 1) {  //pgm gusanos
    return pasos_gusanos;
  }
}
//_____________________________________________________________________________________________

long delay_us_calc(long rpm_) {
  double d_delay_us;
  long _delay_us;
  double factorCorreccion;
  factorCorreccion = 302.1 * pow(rpm_, -0.447); //el valor teórico de delay no resulta en la velocidad calculada (y por lo tanto, en el tiempo de llenado calculado)
                                                //esta función ajusta el delay para obtener la velocidad deseada. En definitiva, calcula un delay menor que el teórico para 
                                                //compensar el tiempo de ejecución de cada loop del programa
  d_delay_us = ((60 * MICROSEC_PER_SEC) / (rpm_ * STEPS_PER_FULL_ROT * STEP_MODE)) / 2;
  d_delay_us = d_delay_us - factorCorreccion;
  _delay_us = d_delay_us;
  return _delay_us;
}

//_____________________________________________________________________________________________

void update_lcd() {
  //first line LCD ------------------------------------
  lcd.setCursor(0, 0);
  lcd.print(menu_number_1);
  lcd.print("|");
  lcd.print(menu[menu_number_1].name_);
  if (menu[menu_number_1].type == 0) {        //if value type
    value_dbl = menu[menu_number_1].value;
    value_dbl = value_dbl / pow(10, menu[menu_number_1].decimals);
    dtostrf(value_dbl, VALUE_MAX_DIGITS, menu[menu_number_1].decimals, value_str );
    lcd.print(value_str); //print value
    lcd.print("          ");
    lcd.setCursor(LCD_COLUMNS - strlen(menu[menu_number_1].suffix), 0);
    lcd.print(menu[menu_number_1].suffix);
  }  else if (menu[menu_number_1].type == 1) { //if option type
    lcd.print(menu[menu_number_1].options[menu[menu_number_1].value]); //print menu[x].option[] of menu[x].value
  } else if (menu[menu_number_1].type == 2) { //if action type
    if (menu_number_1 == 0) { //if Start
      //      lcd.print(" ");
      lcd.print(menu[6].options[menu[6].value]);
    }
  }
  lcd.print("          ");
  //second line LCD ------------------------------------
  lcd.setCursor(0, 1);
  lcd.print(menu_number_2);
  lcd.print("|");
  lcd.print(menu[menu_number_2].name_);
  if (menu[menu_number_2].type == 0) {        //if value type
    value_dbl = menu[menu_number_2].value;
    value_dbl = value_dbl / pow(10, menu[menu_number_2].decimals);
    dtostrf(value_dbl, VALUE_MAX_DIGITS, menu[menu_number_2].decimals, value_str );
    lcd.print(value_str); //print value
    lcd.print("          ");
    lcd.setCursor(LCD_COLUMNS - strlen(menu[menu_number_2].suffix), 1);
    lcd.print(menu[menu_number_2].suffix);
  }  else if (menu[menu_number_2].type == 1) { //if option type
    lcd.print(menu[menu_number_2].options[menu[menu_number_2].value]); //print menu[x].option[] of menu[x].value
  } else if (menu[menu_number_2].type == 2) {  //if action type
    if (menu_number_2 == 0) { //if Start
      lcd.print(menu[6].options[menu[6].value]);
    }
  }
  lcd.print("          ");
  lcd.setCursor(1, 0);
}
//_____________________________________________________________________________________________

boolean encoder_selection(int & x, int lim) { //sub menu
  if (value > last) {
    x++;
    if (x > lim) {
      x = 0;
    }
    last = value;
    return true;
  } else if (value < last) {
    x--;
    if (x < 0) {
      x = lim;
    }
    last = value;
    return true;
  } else {
    return false;
  }
}
//_____________________________________________________________________________________________

boolean encoder_value_selection(int & x, int lim) { //sub menu
  if (value != last) {
    x = x + value - last;
    if (x > lim) {
      x = 0;
    }
    if (x < 0) {
      x = lim;
    }
    last = value;
    return true;
  } else {
    return false;
  }
}
//_____________________________________________________________________________________________

boolean encoder_selection(int & x, int & y, int lim) { //main menu
  if (value > last) {
    x++;
    y++;
    if (x > lim)
    {
      x = 0;
    }
    if (y > lim)
    {
      y = 0;
    }
    last = value;
    return true;
  } else if (value < last) {
    y = menu_number_1;
    x--;
    if (x < 0)
    {
      x = lim;
    }
    last = value;
    return true;
  } else {
    return false;
  }

}
//_____________________________________________________________________________________________

void eepromWriteInt(int adr, int wert) {
  //http://shelvin.de/eine-integer-zahl-in-das-arduiono-eeprom-schreiben/
  byte low, high;

  low = wert & 0xFF;
  high = (wert >> 8) & 0xFF;
  EEPROM.write(adr, low); // dauert 3,3ms
  EEPROM.write(adr + 1, high);
  return;
} //eepromWriteInt
//_____________________________________________________________________________________________

int eepromReadInt(int adr) {
  //http://shelvin.de/eine-integer-zahl-in-das-arduiono-eeprom-schreiben/
  byte low, high;

  low = EEPROM.read(adr);
  high = EEPROM.read(adr + 1);
  return low + ((high << 8) & 0xFF00);
} //eepromReadInt

//_____________________________________________________
int timeToRPM() {
  int rpm;
  double rpm_dbl;
  rpm_dbl = 60 / (STEPS_PER_FULL_ROT * STEP_MODE * double(menu[1].value)); //usé 600 en lugar de 60 así no tenía que dividir el tiempo(menu1.value) por 10
  if (menu[6].value == 0) {  //PGM es moscas
    rpm = int(rpm_dbl * double(menu[8].value) * 10);
  } else if (menu[6].value == 1) { //PGM es gusanos
    rpm = int(rpm_dbl * double(menu[9].value) * 10);
  }
  if (rpm <= VALUE_MAX_RPM) {
    return rpm;
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("  supera max RPM");
    lcd.setCursor(0, 1);
    lcd.print("  autoajustando ");
    delay(3000);
    menu[2].value = VALUE_MAX_RPM;
    menu[1].value = rpmToTIME();
    return VALUE_MAX_RPM;
  }
}

int rpmToTIME() {
  int tiempo;
  double tiempo_dbl;
  tiempo_dbl = 60 / (STEPS_PER_FULL_ROT * STEP_MODE * double(menu[2].value));  //usé 600 en lugar de 60 así no tenía que dividir el rpm(menu2.value) por 10
  if (menu[6].value == 0) {  //PGM es moscas
    tiempo = int(tiempo_dbl * double(menu[8].value) * 10);
  } else if (menu[6].value == 1) { //PGM es gusanos
    tiempo = int(tiempo_dbl * double(menu[9].value) * 10);
  }
  return tiempo;
}

void led() {
  digitalWrite(LEDdosifica, LOW);
  digitalWrite(LEDbombea, LOW);
  digitalWrite(LEDcalibra, LOW);
  if (menu[5].value == 0) {
    digitalWrite(LEDdosifica, HIGH);
  } else if (menu[5].value == 1) {
    digitalWrite(LEDbombea, HIGH);
  } else if (menu[5].value == 2) {
    digitalWrite(LEDcalibra, HIGH);
  }
}
