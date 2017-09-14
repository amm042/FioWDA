/*

  Xbee Analog Sender
  Alan Marchiori - 2017-09-13
  570 Labs LLC
*/
#include <XBee.h>
#define htons(A) ((((uint16_t)(A) & 0xff00) >> 8) | (((uint16_t)(A) & 0x00ff) << 8))

uint16_t myaddress = htons(1870); 
uint16_t mypan = htons(3332);
uint8_t mych = 12;
AtCommandRequest at_my = AtCommandRequest( (uint8_t*)"MY", (uint8_t*)&myaddress, 2);
AtCommandRequest at_ch = AtCommandRequest( (uint8_t*)"CH", (uint8_t*)&mych, 1);
AtCommandRequest at_id = AtCommandRequest( (uint8_t*)"ID", (uint8_t*)&mypan, 2);
AtCommandRequest at_read_my = AtCommandRequest( (uint8_t*)"MY");
AtCommandRequest at_read_ch = AtCommandRequest( (uint8_t*)"CH");
AtCommandRequest at_read_id = AtCommandRequest( (uint8_t*)"ID");
AtCommandRequest at_read_mm = AtCommandRequest( (uint8_t*)"MM");
uint8_t do_init = 1;

static const uint8_t analog_pins[] = {A0,A1,A2,A3,A4,A5,A6,A8};
#define NUM_ADC sizeof(analog_pins)/sizeof(analog_pins[0])

typedef struct {
  uint32_t ticks;
  uint16_t adc_counts[NUM_ADC];
} adc_msg_t;

adc_msg_t msg;
Tx16Request tx = Tx16Request(1874, (uint8_t *)&msg, sizeof(msg));
TxStatusResponse txStatus = TxStatusResponse();

XBee xbee = XBee();

void checkSend(){
  if (xbee.readPacket(1000)) {
    if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
      xbee.getResponse().getTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getStatus() == SUCCESS) {
        // success.  time to celebrate
        Serial.println("tx: success\n");
      } 
      else {
        // the remote XBee did not receive our packet. is it powered on?
        Serial.println("tx: no ack\n");
      } 
    }else if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE){
      AtCommandResponse atResponse = AtCommandResponse();
      xbee.getResponse().getAtCommandResponse(atResponse);

      if (atResponse.isOk()) {
        Serial.print("Command [");
        Serial.print((char)atResponse.getCommand()[0]);
        Serial.print((char)atResponse.getCommand()[1]);
        Serial.println("] was successful!");

        if (atResponse.getValueLength() > 0) {
          Serial.print("Command value length is ");
          Serial.println(atResponse.getValueLength(), DEC);

          Serial.print("Command value: ");
          
          for (int i = 0; i < atResponse.getValueLength(); i++) {
            Serial.print(atResponse.getValue()[i], HEX);
            Serial.print(" ");
          }

          Serial.println("");
        }
      }
    } 
    else {
      Serial.println("tx: got unknown response");
      Serial.println(xbee.getResponse().getApiId());
      Serial.println("\n");
    }
  } 
  else if (xbee.getResponse().isError()) {
    Serial.println("Error reading packet: ");
    Serial.println(xbee.getResponse().getErrorCode());
    Serial.println("\n");
  } 
  else{
    Serial.println("Unknown error\n");
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN_RX, OUTPUT);
  pinMode(LED_BUILTIN_TX, OUTPUT);
  RXLED0;
  TXLED0;

  pinMode(6, OUTPUT);
  pinMode(4, INPUT);
  digitalWrite(6, HIGH);
  
  for (int i =0; i<NUM_ADC; i++){
    pinMode(analog_pins[i], INPUT);
  }
  analogReference(INTERNAL);

  Serial.begin(9600);       // terminal
  Serial1.begin(38400);     // XBee
  xbee.setSerial(Serial1);  // bind xbee to port

  delay(2000);
  xbee.send(at_my);
  checkSend();
  xbee.send(at_ch);
  checkSend();
  xbee.send(at_id);
  checkSend();    
  xbee.send(at_read_my);
  checkSend();  
  xbee.send(at_read_ch);
  checkSend();    
  xbee.send(at_read_id);
  checkSend();    
  xbee.send(at_read_mm);
  checkSend();     
}

void readAdc(){
  msg.ticks= millis();
  for (int i=0; i<NUM_ADC; i++){
    msg.adc_counts[i] = analogRead(analog_pins[i]);
  }
}


// the loop function runs over and over again forever
void loop() {

  delay(1000);
  readAdc();
  Serial.print(msg.ticks);
  Serial.print(": ");
  for (int i=0; i<NUM_ADC;i++){
    
    Serial.print(msg.adc_counts[i]);
    Serial.print(" ");
  }
  Serial.println();
  TXLED1;
  xbee.send(tx);
  checkSend();
  TXLED0;
  
  //delay(1000);                       // wait for a second
}
