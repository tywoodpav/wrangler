
#include <RH_RF69.h>


//#include <SoftwareSerial.h>

#include <TinyGPS.h>
#define VBATPIN A7
//RADIO STUFF
//RADIO SETUP
#define RF69_FREQ 915.0
#define RFM69_CS 8
#define RFM69_INT 3
#define RFM69_RST 4

int cowId = 1;

RH_RF69 rf69(RFM69_CS, RFM69_INT);

struct LocPayload {
  int Id;
  float Battery;
  float Lat;
  float Lon;
};

LocPayload LLoad;

union PayBuff {
  LocPayload LocLoad;
  char Buff[sizeof(LLoad)];
};

PayBuff PBuff;

uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void SendPacket(float latData, float lonData, float battery){
  PBuff.LocLoad.Lat = latData;
  PBuff.LocLoad.Lon = lonData;
  PBuff.LocLoad.Id = cowId;
  PBuff.LocLoad.Battery = battery;
  rf69.send((uint8_t *)PBuff.Buff, sizeof(LocPayload));
  rf69.waitPacketSent();
  return;
}


/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 3(rx) and 4(tx).
*/

TinyGPS gps;
//SoftwareSerial ss(3, 4);
int dataPin = 6;
int safePin = 5;
int alarmPin = A2;
int batteryCounter = 0;
float measuredvbat = 0;

void setup()
{
  pinMode(safePin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(alarmPin, OUTPUT);
  Serial.begin(9600);
  Serial1.begin(9600);


  //RADIO INIT
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  if(!rf69.init()){
    Serial.println("RFM69 radio init fialed"); //MESSAGE IF WE FAILED
    while(1);
  }

  rf69.setModemConfig(RH_RF69::GFSK_Rb250Fd250);
  //rf69.setTxPower(20, true); PETES CODE
  rf69.setTxPower(20);
  
  uint8_t key[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
          0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);

  Serial.print("RDM69 radio @"); Serial.print((int)RF69_FREQ); Serial.println(" MHz");

  
  Serial.print("Simple TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  Serial.println("by Mikal Hart");
  Serial.println();
}

void loop()
{
  
  if(batteryCounter % 50 == 1){
    measuredvbat = analogRead(VBATPIN);
    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage
    Serial.print("VBat: " ); Serial.println(measuredvbat);
  }
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (Serial1.available())
    {
      char c = Serial1.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    digitalWrite(dataPin, HIGH);
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print(" PREC=");
    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
    SendPacket(flat, flon, measuredvbat);
    if(flat > 32.231709 || flat < 32.230429){
      analogWrite(alarmPin, 128);
      digitalWrite(safePin, LOW);
    }
    else if(flon > -110.949849 || flon < -110.952059){
      analogWrite(alarmPin, 128);
      digitalWrite(safePin, LOW);
    }
    else{
      digitalWrite(alarmPin, LOW);
      digitalWrite(safePin, HIGH);
    }
  }
  else{
    digitalWrite(dataPin, LOW);
  }
  
  gps.stats(&chars, &sentences, &failed);
  Serial.print(" CHARS=");
  Serial.print(chars);
  Serial.print(" SENTENCES=");
  Serial.print(sentences);
  Serial.print(" CSUM ERR=");
  Serial.println(failed);
  batteryCounter++;
}



