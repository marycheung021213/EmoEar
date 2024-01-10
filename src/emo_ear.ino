#define USE_ARDUINO_INTERRUPTS true

#include <Servo.h>
#include <PulseSensorPlayground.h>

// Pulse Sensor
const int PulseWire = A0;
int Threshold = 550;

// AdjustPulseSensor
int count=0;
int test_heartrate[10];
int u=0;
int IdealAverage=70;
int BPM_current=0;
int BPM_former=0;

// Servo
int pos = 0;
int minus=0;
int pos_9=minus-pos;


// ultrasonic
const int TrigPin=3;                                                                                                      
const int EchoPin=4;
float distance;
boolean distance_judge=false;
int count2=0;
int count3=0;

// buzzer
int buzzer=12;

//LED
const int redPin = A1; // 红色引脚
const int greenPin = A3; // 绿色引脚
const int bluePin = A5; // 蓝色引脚

// Initialize
Servo servo_9;
Servo servo_10;
PulseSensorPlayground pulseSensor;


void setup() {
  // Open Monitor
  Serial.begin(9600);

  // PulseSensor
  pulseSensor.analogInput(PulseWire);
  pulseSensor.setThreshold(Threshold);   
  
  if (pulseSensor.begin()) {
    Serial.println("Pulse Sensor Start!");\
  }
  
  // ultrasonic
  pinMode(TrigPin,OUTPUT);
  pinMode(EchoPin,INPUT);

  // buzzer
  pinMode(buzzer,OUTPUT);

  // Servo
  servo_9.attach(9, 500,2500);
  servo_10.attach(10, 500,2500);
  
  servo_9.write(0);
  servo_10.write(30);

}

///////// Main Loop /////////

void loop() {
  
  // Heartrate
 BPM_current= AdjustPulseSensor();

 if(BPM_current==0){
  BPM_current=BPM_former;
 }

  if(count>10  && Ultrasonic()){
    if(BPM_current<80){
      sweep2();
      show(0);
    }else{
      if(BPM_current<110){
        sweep1();
        show(1);
      }else{
        swing();
        show(2);
      }
    }
  }

  BPM_former=BPM_current;
}


// Heartrate
int AdjustPulseSensor(){
  
  int BPM=pulseSensor.getBeatsPerMinute(); //PulseSensorPlayground自带函数

  if(pulseSensor.sawStartOfBeat() && BPM<130 && BPM>50){
    count+=1; //计数器
    if(count<=5){
      /*
       * 经过多次试验观测得出，前5次测量数据(count 1~5)非常不稳定，建议摒弃
       */
       Serial.print("Adjusting……");
       Serial.println(count);
       Serial.println("-------------------------------");
    }else{
      if(count<=10){
        /*
         * 前5到前10次数据(count 06~10)作为测试数据，以得出本次测量的平均心率
         */
         test_heartrate[count-6]=BPM;
         Serial.println("Testing……");
         Serial.print("Test BPM:");
         Serial.println(BPM);
         Serial.println("-------------------------------");
      }else{
        if(count==11){
          /*
           * 从第11次开始测量，数据可以正式使用
           */
           for(int i=0;i<5;i++){
            u+=test_heartrate[i];
           }
           u=u/5;
           Serial.print("Test finished!Your average heartrate is:");
           Serial.println(u);
           Serial.print("Your BPM:");
           Serial.println(BPM-u+IdealAverage);
           Serial.println("-------------------------------");
        }else{
          Serial.print("Your BPM:");
          Serial.println(BPM-u+IdealAverage);
          Serial.println("-------------------------------");
        }
      }
    }
  }else{
    return 0;
  }

  delay(20);

  return BPM-u+IdealAverage;
}

//ultrasonic
boolean Ultrasonic(){
  
  // ultrasonic activation
  digitalWrite(TrigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(TrigPin,HIGH);
  delayMicroseconds(2);
  digitalWrite(TrigPin,LOW);

  // distance
  distance=pulseIn(EchoPin,HIGH)/58.00;
  Serial.print(distance);
  Serial.print("cm");
  Serial.println();

  if(distance<100){
      distance_judge=true;
    }else{
      distance_judge=false;
    }
  if(distance_judge && count3<5 ){
    digitalWrite(buzzer, LOW);//发声
    delay(200);
    
    digitalWrite(buzzer, HIGH);//不发声

    digitalWrite(buzzer, LOW);//发声
    delay(200);
    
    digitalWrite(buzzer, HIGH);//不发声
    
    count3+=1;

    servo_9.write(40);
    servo_10.write(10);
    show(3);
    return false;
  }else{
    return true;
  }

  return true;
}

//LED
void show(int mode){
  if(mode==0){
    setColor(0xff, 0xff, 0xff);//白色
    setColor(0, 0xff, 0xff); // 青色
  }
  if(mode==1){
    setColor(0xff, 0xff, 0xff);//白色
    setColor(0, 0, 0xff); // 蓝色
  }
  if(mode==2){
    setColor(0xff, 0xff, 0xff);//白色
    setColor(0, 0xff, 0); // 绿色
  }
  if(mode==3){
    setColor(0xff, 0xff, 0xff);//白色
    setColor(0, 0xff, 0x88); 
  }
}

// Servo
void sweep2(){
  for (pos = 0; pos <= 30; pos += 1) {
    servo_10.write(pos);
    servo_9.write(30-pos);
    delay(15);
  }
  for (pos = 30; pos >= 0; pos -= 1) {
    servo_10.write(pos);
    servo_9.write(30-pos);
    delay(15);
  }
}

void sweep1(){
  for (pos = 0; pos <= 30; pos += 3) {
    servo_9.write(pos);
    servo_10.write(30-pos);
    if(pos==30){
        delay(1000);
    }
    delay(15);
  }
    servo_9.write(0);
    servo_10.write(30);
}

void swing(){
  for (pos = 30; pos <= 90; pos += 15) {
    servo_10.write(pos);
    delay(500);
  }
  for (pos = 90; pos >= 30; pos -= 15) {
    servo_10.write(pos);
    delay(500);
  }
}


// 设置LED的颜色
void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif

  // 设置RGB针脚的电压，有效值0-255，对应0-5V电压
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}
