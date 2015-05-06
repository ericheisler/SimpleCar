// the remote for theo's robot

// pins
#define txpin 1
#define lButton 3
#define rButton 4

// for IR protocol
#define bitthreshold 1500
#define headerOn 9000
#define headerOff 4500
#define pulseLength 562
#define zeroBit 562
#define oneBit 1687
#define txOn TCCR1 = (1<<CTC1)|(1<<COM1A0)|(1<<CS10)
#define txOff TCCR1 = (1<<CTC1)|(1<<CS10)

// some numbers
#define delayTime 150
#define LEFTGO 1
#define RIGHTGO 2
#define BOTHGO 3

// for delaying between transmissions
unsigned long sendDelay;
uint8_t message;

void setup(){
  pinMode(lButton, INPUT_PULLUP);
  pinMode(rButton, INPUT_PULLUP);
  
  // IR
  pinMode(txpin, OUTPUT);
  digitalWrite(txpin, LOW);
  // 38kHz carrier, use CTC with top 104
  //TCCR1 = (1<<CTC1)|(1<<COM1A0)|(1<<CS10); // this turns the output on
  TCCR1 = (1<<CTC1)|(1<<CS10); // this turns it off
  OCR1C = 104;
  
  sendDelay = 0;
  message = 0;
}

void loop(){
  while(millis() - sendDelay < delayTime); //wait for delay
  //check button state and send message
  message = 0;
  if(digitalRead(lButton) == LOW){
    message = LEFTGO;
  }
  if(digitalRead(rButton) == LOW){
    if(message){
      message = BOTHGO;
    }else{
      message = RIGHTGO;
    }
  }
  if(message){
    transmit(message);
    sendDelay = millis();
  }
}

void transmit(uint8_t output){
  irSendHeader();
  irSendByte(output);
  irSendStopBit();
}

void irSendHeader(){
  // header
  txOn;
  delayMicroseconds(headerOn);
  txOff;
  digitalWrite(txpin, LOW);
  delayMicroseconds(headerOff);
}

void irSendByte(uint8_t output){
  for(int8_t i=7; i>=0; i--){
    if(output & (1<<i)){
      txOn;
      delayMicroseconds(pulseLength);
      txOff;
      digitalWrite(txpin, LOW);
      delayMicroseconds(oneBit);
    }else{
      txOn;
      delayMicroseconds(pulseLength);
      txOff;
      digitalWrite(txpin, LOW);
      delayMicroseconds(zeroBit);
    }
  }
}

void irSendStopBit(){
  // stop bit
  txOn;
  delayMicroseconds(pulseLength);
  txOff;
  digitalWrite(txpin, LOW);
  delay(1);
}
