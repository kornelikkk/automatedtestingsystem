
#include <SoftwareSerial.h>
#include <GyverPlanner2.h>
#include <EEPROM.h>
//#define GS_FAST_PROFILE 100

int32_t StepsPerRev = 1600;

// Координаты вращения головы
int32_t path[][3] = {
  {0, 0, 0},
  {-43000, -1600, -16000},
  {0, 0, 0}
};

//int nodeAmount = sizeof(path) / 12; // Количество точек. Определяется как вес всего массива / (4*3) байта
int nodeAmount = 3; // Количество точек. Определяется как вес всего массива / (4*3) байта

//Объявление концевиков
const int conPin0 = 2;
const int conPin1 = 3;
const int conPin2 = 4;

// Объявление дисплея
SoftwareSerial serialDisplay(26, 27); //rx tx


int16_t adress = 0;

#define max_adress 4095 //uno == 1023     mega == 4095
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

  // Шаговики
  planner.setAcceleration( 400 );      // Ускорение
  planner.setMaxSpeed( 2400 );         // Максимальная скорость
  
  // Настройка планировщика
  planner.addStepper(0, stepper1);  // ось 0
  planner.addStepper(1, stepper2);  // ось 1
  planner.addStepper(2, stepper3);  // ось 2

  // Концевики
  pinMode(conPin0, INPUT);
  pinMode(conPin1, INPUT);
  pinMode(conPin2, INPUT);
  //pinMode(13, OUTPUT);

  homing(); // Приходим в нулевые координаты (до концевиков)
  planner.setCurrent(path[0]); // Установка нулевых координат
  planner.start();
 
  k = EEPROM.read(4094);
  if(k!=0){
    for (int i = 0; i<max_adress-4; i += 2)
      EEPROM.write(i,0);
  }
  while(true)
  {
    if (EEPROM.read(adress+2) >= max_cycle)
    {
      adress+=4;
    }else
    {
      break;
    }
  }
  displayCounterOn();
  //digitalWrite(13, 1);
  planner.tick();
  planner.addTarget(path[j], 0, ABSOLUTE);
      if ( ++j >= nodeAmount ) {
        j = 0; // Закольцевать
        
      }
}



//-----------------------------Шаговики------------------------------------------------
//Основное управление шаговиками
//int count = 0; // Счётчик количества проворотов головы

bool first_on = true;
void loop() {
  if (first_on)
    {
      first_on=false;
      delay(10000);
    }
  planner.tick(); // Тикер для управления шаговиками
//  Serial.println(digitalRead(conPin0));
  
  if (planner.available()) {
    // добавляем точку маршрута и является ли она точкой остановки (0 - нет)
      planner.addTarget(path[j], 0, ABSOLUTE);
      if ( ++j >= nodeAmount ) {
        j = 0; // Закольцевать
        //++count; //Глобальный отсчёт
        displayCounter(); //Глобальный отсчёт
        Serial.print(nodeAmount);
      }
  }

  // Дисплей
  //if(false) {
  //    displayCounter();//очень прошу быть осторожным, кол-во записаей ограничено. 
  //    //Память можно перезаписать 10000^(1022/4) раз. 
  //    //если будет бесконечный цикл, то eeprom сломается
  //  };
}

void yield() {
    planner.tick(); // Тикер для управления шаговиками
//  Serial.println(digitalRead(conPin0));
  
  if (planner.available()) {
    // добавляем точку маршрута и является ли она точкой остановки (0 - нет)
      planner.addTarget(path[j], 0, ABSOLUTE);
      if ( ++j >= nodeAmount ) {
        j = 0; // Закольцевать
        //++count; //Глобальный отсчёт
      }
  }
}

//--------------Концевики---------------------------------------------------------------
// Проверка концевиков
void homing() {
  if (digitalRead(conPin0)) {       // если концевик 0 не нажат
    planner.setSpeed(0, 1600);       // ось 0
    while (digitalRead(conPin0)) {  // пока кнопка не нажата
      planner.tick();               // крутим
    }
    // кнопка нажалась - покидаем цикл
    planner.brake();                // тормозим, приехали
  }

  //  if (digitalRead(conPin1)) {   
  //    planner.setSpeed(1, 1600);       
  //    while (digitalRead(conPin1)) planner.tick();
  //    planner.brake();                
  //  }

  if (digitalRead(conPin2)) {       
    planner.setSpeed(2, 800);       
    while (digitalRead(conPin2)) planner.tick();               
    planner.brake();                
  }

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
  int16_t a = EEPROM.read(adress);
  String str = String(a);
  serialDisplay.print("page0.counter.val=" + str);
  comandEnd();
}
void displayCounter(){ 
  int16_t a = EEPROM.read(adress);
  int16_t b = EEPROM.read(adress + 2);
  if (b >= max_cycle)
  {
     if (adress + 4 >= 4096)
     {
       if(true){
          for (int i = 0; i<max_adress; i += 2)
            EEPROM.write(i,0);
        }
     }else
     {
      adress += 4;
      b = 0;
     }
  }
  if (a >= max_value-1)
  {
    a = 0;
    b++;
     EEPROM.update(adress, 0);
     EEPROM.update(adress + 2, b);
    String str = String(a);
    serialDisplay.print("page0.counter.val=" + str);
    comandEnd();
    while(true){}
  }
  a++;
  b++;
   EEPROM.update(adress, a);
   EEPROM.update(adress + 2, b);
  String str = String(a);
  serialDisplay.print("page0.counter.val=" + str);
  comandEnd();
}