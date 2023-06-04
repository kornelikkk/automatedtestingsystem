#include <SoftwareSerial.h>
#include <GyverPlanner2.h>
#include <EEPROM.h>

int32_t StepsPerRev = 1600;

// Координаты вращения головы
int32_t path[][3] = {
  {0, 0, 0},
  {-43000, -1600, -16000},
  {0, 0, 0}
};

int nodeAmount = sizeof(path) / 12; // Количество точек. Определяется как вес всего массива / (4*3) байта
//int nodeAmount = 3; // Количество точек. Определяется как вес всего массива / (4*3) байта

//Объявление концевиков
const int conPin0 = 3;
const int conPin1 = 4;
const int conPin2 = 2;

// Объявление дисплея
SoftwareSerial serialDisplay(26, 27); //rx tx

int16_t adress = 0;

#define max_adress 4094 //uno == 1023     mega == 4095
#define max_cycle 100000
#define max_value 1000000


//Объявление шаговиков
Stepper<STEPPER2WIRE> stepper1(5, 6); 
Stepper<STEPPER2WIRE> stepper2(8, 9);
Stepper<STEPPER2WIRE> stepper3(11, 12);
GPlanner2<STEPPER2WIRE, 3> planner;


int j = 0; // Счётчик точек маршрута


void setup() {
  Serial.begin(9600);
  serialDisplay.begin(9600);

 // Экран
  displayCounterOn();

  // Шаговики
  planner.setAcceleration( 300 );      // Ускорение
  planner.setMaxSpeed( 1800 );         // Максимальная скорость

  // Настройка планировщика
  planner.addStepper(0, stepper1);  // ось 0
  planner.addStepper(1, stepper2);  // ось 1
  planner.addStepper(2, stepper3);  // ось 2

  // Концевики
  pinMode(conPin0, INPUT);
  pinMode(conPin1, INPUT);
  pinMode(conPin2, INPUT);

  // enPin
  //pinMode(7, OUTPUT);
  //pinMode(10, OUTPUT);
  //pinMode(13, OUTPUT);

  homing();
  planner.setCurrent(path[0]); // Установка нулевых координат
  planner.start();

  //planner.tick();
  //planner.addTarget(path[j], 0, ABSOLUTE);
  //    if ( ++j >= nodeAmount ) {
  //      j = 0; // Закольцевать      
  //    }
}

//-----------------------------Шаговики------------------------------------------------

bool first_on = true;
void loop() {

  if (first_on){
      first_on=false;
      delay(1000);
  }

  planner.tick(); // Тикер для управления шаговиками

  if (planner.available()) {
    // добавляем точку маршрута и является ли она точкой остановки (0 - нет)
      planner.addTarget(path[j], 0, ABSOLUTE);
      if ( ++j >= nodeAmount ) {
        j = 0; // Закольцевать
        displayCounter(); //Глобальный отсчёт
      }
  }
}

void yield() {
  //planner.tick(); 

  if (planner.available()) {
      planner.addTarget(path[j], 0, ABSOLUTE);
      if ( ++j >= nodeAmount ) {
        j = 0;
      }
  }
}


//--------------Концевики---------------------------------------------------------------

void homing() {
  int count_conPin0 = 0;
  int count_conPin2 = 0;

  planner.setSpeed(0, 600);      // ось 0
      
  while ( count_conPin0 < 15000 ) {
    planner.tick();
    if ( digitalRead( conPin0 ) == 0 ) ++count_conPin0;
  }

  planner.brake();

  planner.setSpeed(1, -900);
  while ( count_conPin2 < 3000 ) {
    planner.tick();
    if ( digitalRead( conPin2 ) == 0 ) ++count_conPin2;
  }

  planner.brake();                // тормозим, приехали

  planner.reset();    // сбрасываем координаты в 0
}



//--------------Дисплей---------------------------------------------------------------
// Отображение на дисплей (от Али)

void comandEnd() {
  for (int i = 0; i < 3; i++) {
    serialDisplay.write(0xff);
  }
}
void displayCounterOn()
{
  long k;
  EEPROM.get(max_adress-4,k);
  if(k!=0){
    for (int i = 0; i<max_adress+1; i += 1)
     { 
      EEPROM.write(i,0);
     }
  }
  int adr;
  while(true)
  {
    EEPROM.get(adress+4, adr);
    if (adr >= max_cycle)
    {
      adress+=6;
    }else
    {
      break;
    }
  }
  long a;
  EEPROM.get(adress,a);
  String str = String(a);
  serialDisplay.print("page0.counter.val=" + str);
  comandEnd();
}
void displayCounter(){ 
  long a;
  EEPROM.get(adress,a);
  Serial.print(a);
  int16_t b;
  EEPROM.get(adress + 4, b);
  if (b >= max_cycle-1)
  {
     if (adress + 6 >= max_adress-6)
     {
      for (int i = 0; i<max_adress+1; i += 1)
      {
        EEPROM.write(i,0);
      }
      adress = 0;
      EEPROM.put(adress, a);
     }else
     {
      adress += 6;
      b = 0;
      a++;
      EEPROM.put(adress, a);
      EEPROM.put(adress + 4, b);
      String str = String(a);
      serialDisplay.print("page0.counter.val=" + str);
      comandEnd();
      return;
     }
  }
  if (a >= max_value)
  {
    b++;
    EEPROM.put(adress, 0);
    EEPROM.put(adress + 4, b);
    String str = String(a);
    serialDisplay.print("page0.counter.val=" + str);
    comandEnd();
    while(true){ }
  }
  a++;
  b++;
  EEPROM.put(adress, a);
  EEPROM.put(adress + 4, b);
  String str = String(a);
  serialDisplay.print("page0.counter.val=" + str);
  comandEnd();
}