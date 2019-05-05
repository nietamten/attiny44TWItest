
#include <Wire.h>

uint8_t regSizes[] =  { 2,    2,    2,    1,    4 }; //max 4, kopia w i2cslave i lua

uint8_t cmd[15];

uint8_t appendCmd(uint8_t c)
{
  uint8_t res = 0;
  
  if(c=='r' || c=='w')
  {
    memset(cmd,0,sizeof(cmd));
    cmd[0] = c;
    res = 0;
  }
  else
  {  
    for(uint8_t i=0;i<sizeof(cmd);i++)
    {
      if(cmd[i] == 0)
      {
        cmd[i]=c;
        res = i;
        break; 
      }
    }
  }
  return res;
}

void printHEX(uint8_t v){
  Serial.print(v>>4&0x0f, HEX);
  Serial.print(v&0x0f, HEX);
}

void exe(uint8_t cnt)
{
  if( cnt >= sizeof(cmd)-1 || (cmd[0] != 'r' && cmd[0] != 'w') )
  {
      memset(cmd,0,sizeof(cmd));      
      return;
  }
  else if(cnt>=4)
  {

    char buff[3];
    buff[0] = cmd[1];
    buff[1] = cmd[2];
    buff[2] = 0;
    uint8_t dev = strtol(buff,NULL,16); 
    buff[0] = cmd[3];
    buff[1] = cmd[4];
    uint8_t addr = strtol(buff,NULL,16); 
    
    if(addr>=sizeof(regSizes))
    {
      Serial.print("!");
      return;
    }
       
    if(cmd[0] == 'w' && cnt == 4+(2*regSizes[addr]) )
    {
      uint8_t v[4];
      for(int i=0;i<regSizes[addr];i++)
      {
        buff[0] = cmd[5+(2*i)];
        buff[1] = cmd[6+(2*i)];
        v[i] = strtol(buff,NULL,16);
      }

      //printHEX(dev); 
      //printHEX(addr);
      Wire.beginTransmission(dev); 
      Wire.write(addr);
      for(int i=0;i<regSizes[addr];i++)
      {
        printHEX(v[i]);
        Wire.write(v[i]);
      }
      Wire.endTransmission(false);     
      Serial.print("*");

      memset(cmd,0,sizeof(cmd));
    }
    if(cmd[0] == 'r' && cnt == 4)
    {
      Wire.beginTransmission(dev); 
      Wire.write(addr);
      Wire.endTransmission(false);  
      Wire.requestFrom(dev, regSizes[addr]);
      while (Wire.available()) {
        printHEX(Wire.read()); 
      }
      Serial.print("*");     

      memset(cmd,0,sizeof(cmd));              
    }
  }
}

void setup() {

//leds out
//DDRB |= 1<<PB0;
//DDRD |= 1<<PD5;

memset(cmd,0,sizeof(cmd));

Serial.begin(115200);
while (!Serial) {}
//Serial.print("!");

Wire.begin(1);//TWI on
}

void loop() {

  while (Serial.available() > 0) {
      exe(appendCmd(Serial.read()));
  }

}
