/* Solune Receiver Firmware
    by Peter Valentine
	The Receiver is located with the Solune Lights themselves, they are the spokes in a Spoke and Hub network
*/

#include <SPI.h>
#include <RH_RF69.h>

//*** Radio Setup ***
#define RF69_FREQ 915.0     // Change to 434.0 or other frequency, must match RX's freq!
#define RFM69_CS      8     //Feather M0 Setup
#define RFM69_INT     3
#define RFM69_RST     4
#define LED           13
RH_RF69 rf69(RFM69_CS, RFM69_INT);  // Singleton instance of the radio driver
int16_t packetnum = 0;  // packet counter, we increment per xmission
float RSSI;     //Received Signal Strength from last Reception

//We put all of the data for the payload into a single structure
struct LocPayload {
  int Id;
  float Battery;
  float Lat;
  float Lon;
};
LocPayload LLoad;

int PBuffInt = 0;
int IdInt = 0;
int LatInt = 0;
int LonInt = 0;
int BatteryInt = 0;

union PayBuff {
  LocPayload LocLoad;
  char Buff[sizeof(LLoad)];
};
PayBuff PBuff;

char result[9];
 
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];     //Our Max Size Buffer
uint8_t len = sizeof(buf);    //The Length of our Buffer


unsigned long StartTime=millis(); //Start Time of a Cycle

boolean ReceivePacket() {

  if (rf69.recv(buf, &len)) //If we have data in the buffer
  {
    //Serial.println("got raw...");
    RSSI = rf69.lastRssi();     //Get RSSI
    if (RSSI > 0) {
      RSSI = RSSI * -1;  //Adjust for Bad Signs
    }
    memcpy(&PBuff, buf, sizeof(PBuff));   //Map data back into PBuff to decode it

    /*
    
    Serial.print("Lat: "); Serial.println(PBuff.LocLoad.Lat, 6); 
    Serial.print("Lon: "); Serial.println(PBuff.LocLoad.Lon, 6); 
    Serial.print("Id:  "); Serial.println(PBuff.LocLoad.Id);
    Serial.print("Battery: "); Serial.println(PBuff.LocLoad.Battery);
    */


    
    /*
    IdInt = PBuff.LocLoad.Id + 1000; //MAKES ID A 4 DIGIT INT, FIRST DIGIT IS 1
    Serial1.write(IdInt);
    delay(50);
    
    BatteryInt = (int)(PBuff.LocLoad.Battery * 100) + 2000; //MAKES BATTER A 4 DIGIT INT, FIRST DIGIT IS 2 
    Serial1.write(BatteryInt);
    delay(50);
    
    LatInt = (int)(PBuff.LocLoad.Lat * 1000000) + 300000000; //MAKES LAT A 9 DIGIT INT, FIRST DIGIT IS 3
    Serial1.write(LatInt);
    delay(50);
    
    LonInt = (int)(PBuff.LocLoad.Lon * 1000000) + 400000000; //MAKES LON A 9 DIGIT INT, First Digit is 4
    Serial1.write(LonInt);
    delay(50);
    */
    
    return true;
  }
  else {
    Serial.println("Receive failed");
    return false;
  }

}

void setup() {
  Serial.begin(9600);   //Use High Speed interface to reduce lag
  Serial1.begin(9600);
  delay(1000);
  Serial.println("Solune Receiver v1.1");


  //*** Radio Init ***
  pinMode(RFM69_RST, OUTPUT);   //Set the Reset Pin on the Radio for output
  digitalWrite(RFM69_RST, LOW); //And set it low
  digitalWrite(RFM69_RST, HIGH); //And perform a reset on startup
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  if (!rf69.init()) {   //Now try to initialize the radio
    Serial.println("RFM69 radio init failed");  //Message if we failed
    while (1);  //And freeze
  }
  rf69.setModemConfig(RH_RF69::GFSK_Rb250Fd250); //Default
  //rf69.setModemConfig(RH_RF69::GFSK_Rb125Fd125);
  //rf69.setModemConfig(RH_RF69::GFSK_Rb38_4Fd76_8);
  //rf69.setModemConfig(RH_RF69::GFSK_Rb2Fd5);
  rf69.setTxPower(20);  // range from 14-20 for power, 2nd arg must be true for 69HCW
  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                  };
  rf69.setEncryptionKey(key);

  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");


}

void loop() {
  if (rf69.available()) {
    ReceivePacket();
  }
    while (Serial1.available()) {
        Serial.write(Serial1.read());
    }

    while (Serial.available()) {
        Serial1.write(Serial.read());
    }

    //sprintf(result, "String value: %d.%02d", (int)PBuff.LocLoad.Lat, (int)(PBuff.LocLoad.Lat*100)%100);
    //Serial.write("Lat: "); Serial.write(result);
    //Wire.write("Lon: "); Wire.write(result);
    //Wire.write("Id: "); Wire.write(result);
    //Wire.write("Battery: "); Wire.write(result);
    delay(100);
}

//Receives an incoming packet of Data





