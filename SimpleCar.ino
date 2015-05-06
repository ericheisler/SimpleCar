// A three wheeled car for the kid

#define lmotor 10
#define rmotor 9
#define irInput 2
#define motorPulseLength 200
#define bitthreshold 1500 // for IR protocol
#define startthreshold 10000 // for IR protocol

unsigned long motorTimer;

// for IR
volatile char irStartReceive;
volatile unsigned long irStartTime;
unsigned long irStopTime, irTimeout;
uint8_t irIn;

ISR(INT0_vect){
  if(!irStartReceive){
    irStartReceive = true;
    irStartTime = micros();
  }
}

void setup(){
  pinMode(lmotor, OUTPUT);
  pinMode(rmotor, OUTPUT);
  
  // IR
  irStartReceive = false;
  pinMode(irInput, INPUT_PULLUP);
  EICRA |= (1<<ISC01);
  EIMSK |= (1<<INT0);
  sei();
}

void loop(){
  if(irStartReceive){
    if(irReceive() == 1){
      // a byte was received
      if(irIn == 1){
        digitalWrite(lmotor, HIGH);
        digitalWrite(rmotor, LOW);
      }else if(irIn == 2){
        digitalWrite(lmotor, LOW);
        digitalWrite(rmotor, HIGH);
      }else if(irIn == 3){
        digitalWrite(lmotor, HIGH);
        digitalWrite(rmotor, HIGH);
      }
      motorTimer = millis();
    }
  }
  
  if(millis() > motorTimer + motorPulseLength){
    stop();
  }
}

void stop(){
  digitalWrite(lmotor, LOW);
  digitalWrite(rmotor, LOW);
}

// receives IR input
// returns 0 = no input, 1 = received,
// 2 = noise, 3 = no rise(stuck)
// 4 = long header(bad timing), 5 = incomplete byte
uint8_t irReceive(){
  // check for incoming signal
  if(irStartReceive){
    // start time has already been set
    // wait for 400us for ???
    delayMicroseconds(400);
    if((PIND&(1<<2)) > 0){
      // it was noise
      irStartReceive = false;
      return 2;
    }
    
    // check the start interval length
    irTimeout = millis();
    while(((PIND&(1<<2)) == 0)){ // wait for rise
      if(millis()-irTimeout > 200){
        irStartReceive = false;
        return 3;
      }
    }
    irTimeout = millis();
    while(((PIND&(1<<2)) > 0)){ // wait for fall
      if(millis()-irTimeout > 200){
        irStartReceive = false;
        return 4;
      }
    }
    irStartTime = micros();
    
    // receive a byte
    if(!irReceiveByte()){
      irWaitForStopBit();
      irStartReceive = false;
      return 5;
    }
    irWaitForStopBit();
    irStartReceive = false;
    return 1;
  }
  // there was no incoming
  return 0;
}

bool irReceiveByte(){
  uint8_t b = 0;
  uint8_t i = 0;
  while(i<8){
    i++;
    b <<= 1;
    irTimeout = millis();
    while(((PIND&(1<<2)) == 0)){ // wait for rise
      if(millis()-irTimeout > 20){
        return false;
      }
    }
    irTimeout = millis();
    while(((PIND&(1<<2)) > 0)){ // wait for fall
      if(millis()-irTimeout > 20){
        return false;
      }
    }
    irStopTime = micros();
    if(irStopTime-irStartTime > bitthreshold){
      b++;
    }
    irStartTime = irStopTime;
  }
  irIn = b;
  return true;
}

void irWaitForStopBit(){
  irTimeout = millis();
  while(((PIND&(1<<2)) == 0)){ // wait for rise
    if(millis()-irTimeout > 20){
      return;
    }
  }
}
