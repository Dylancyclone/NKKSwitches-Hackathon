#include <dht.h>
#include <SPI.h>
#include <MFRC522.h>

dht DHT;
MFRC522 mfrc522(10, 9);

int  left_button_pressed = 0;
int  right_button_pressed = 0;
int  left_button_released = 0;
int  right_button_released = 0;

/*
 * Inputs: None
 * Outputs: None
 * Function: Reboots the SmartSwitch
 */
void SS_reboot(){
  Serial.write(0x24);
}

/*
 * Inputs: None
 * Outputs: 1 if communication check is successful, 0 if communication check is failed
 * Function: Checks if the SmartSwitch is successfully communicating with the Arduino
 */
int SS_comm_check(){
    Serial.write(0x1);
    delay(3);
    
    return Serial.read() == 0x61;
}

/*
 * Inputs: brightness: 8-bit integer from 1-15 representing the desired brightness (15 is brightest)
 * Outputs: 1 if SmartSwitch acknowledges command, 0 if SmartSwitch does not acknowledge command
 * Function: Changes the brightness of the SmartSwitch
 */
int SS_set_brightness(uint8_t brightness){
  int brightnessAsciiHex1 = nibble_to_ascii_hex((brightness & 0xF0) >> 4);
  int brightnessAsciiHex2 = nibble_to_ascii_hex(brightness & 0x0F);
  
  uint8_t msg[] = {0x27, 0x4E, brightnessAsciiHex1, brightnessAsciiHex2, 0x00}; 
  Serial.write(msg, sizeof(msg));
  delay(3);
  
  return Serial.read() == 0x61;
}

/*
 * Inputs: offColor: 16-bit integer representing the background colors of text characters (must be in 565 BGR format)
 *         onColor: 16-bit integer representing the colors of text characters(must be in 565 BGR format)
 * Outputs: 1 if SmartSwitch acknowledges command, 0 if SmartSwitch does not acknowledge command
 * Function: Changes the color of text characters to the onColor and the background of those characters to the offColor
 */
int SS_set_onoff_color(uint16_t offColor, uint16_t onColor){
  int offColorAsciiHex1 = nibble_to_ascii_hex((offColor & 0xF000) >> 12);
  int offColorAsciiHex2 = nibble_to_ascii_hex((offColor & 0x0F00) >> 8);
  int offColorAsciiHex3 = nibble_to_ascii_hex((offColor & 0x00F0) >> 4);
  int offColorAsciiHex4 = nibble_to_ascii_hex(offColor & 0x000F);
  int onColorAsciiHex1 = nibble_to_ascii_hex((onColor & 0xF000) >> 12);
  int onColorAsciiHex2 = nibble_to_ascii_hex((onColor & 0x0F00) >> 8);
  int onColorAsciiHex3 = nibble_to_ascii_hex((onColor & 0x00F0) >> 4);
  int onColorAsciiHex4 = nibble_to_ascii_hex(onColor & 0x000F);

  uint8_t msg[] = {0x27, 0x49, offColorAsciiHex1, offColorAsciiHex2, offColorAsciiHex3, offColorAsciiHex4, onColorAsciiHex1, onColorAsciiHex2, onColorAsciiHex3, onColorAsciiHex4}; 
  Serial.write(msg, sizeof(msg));
  delay(3);
  
  return Serial.read() == 0x61;
}

/*
 * Inputs: color: 16-bit integer representing the color of the line (must be in 565 BGR format)
 * Outputs: 1 if SmartSwitch acknowledges command, 0 if SmartSwitch does not acknowledge command
 * Function: Changes the color of the next line
 */
int SS_set_line_color(uint16_t color){
  int colorAsciiHex1 = nibble_to_ascii_hex((color & 0xF000) >> 12);
  int colorAsciiHex2 = nibble_to_ascii_hex((color & 0x0F00) >> 8);
  int colorAsciiHex3 = nibble_to_ascii_hex((color & 0x00F0) >> 4);
  int colorAsciiHex4 = nibble_to_ascii_hex(color & 0x000F);

  uint8_t msg[] = {0x27, 0x47, colorAsciiHex1, colorAsciiHex2, colorAsciiHex3, colorAsciiHex4}; 
  Serial.write(msg, sizeof(msg));
  delay(3);
  
  return Serial.read() == 0x61;
}

/*
 * Outputs: 1 if SmartSwitch acknowledges command, 0 if SmartSwitch does not acknowledge command
 * Function: clear switch
 */
int SS_clear(uint8_t switchNumber){
  uint8_t switchCommand = 0;
  if(switchNumber == 2)
    switchCommand = 0x5A;
  else
    switchCommand = 0x59;

  uint8_t msg[] = {0x27, switchCommand, 0x34, 0x30, 0x30, 0x30 }; 
  Serial.write(msg, sizeof(msg));
  delay(3);
  
  return Serial.read() == 0x61;
}

/*
 * Inputs: switchNumber: 8-bit integer from 1-2 representing the desired switch (1 for left, 2 for right)
 * Outputs: 1 if SmartSwitch acknowledges command, 0 if SmartSwitch does not acknowledge command
 * Function: Adds the text "ASU" to the selected switch
 */
int SS_add_text(String len, int row, int col, String text, uint8_t switchNumber){
  uint8_t switchCommand = 0;
  if(switchNumber == 2)
    switchCommand = 0x52;
  else
    switchCommand = 0x51;
  //uint8_t msg[] = {0x27, switchCommand, 0x30, 0x33, 0x30, 0x41, 0x30, 0x41, 0x34, 0x31, 0x35, 0x33, 0x35, 0x35}; 
  //Serial.write(msg, sizeof(msg));

  char leng[3];
  strcpy(leng, len.c_str());
  
  char rowHex[3];
  itoa(row,rowHex,16);
  if(rowHex[1]=='\0')
  {
    rowHex[1]=rowHex[0];
    rowHex[0]='0';
    rowHex[2]='\0';
  }
  char colHex[3];
  itoa(col,colHex,16);
  if(colHex[1]=='\0')
  {
    colHex[1]=colHex[0];
    colHex[0]='0';
    colHex[2]='\0';
  }
  
  char string[text.length()+1];
  strcpy(string, text.c_str());
  char buffer[4*sizeof(string)]; //sized for the worst case scenario of each being in the hundreds plus a space between each and a null
  char* buffPtr = buffer;
  
  Serial.print(sizeof(string) - 1); //The program literally wont work without this line.
  for(byte i = 0; i < sizeof(string) - 1; i++){
    itoa((int)string[i],buffPtr,16); //convert the next character to a string and store it in the buffer
    buffPtr += strlen(buffPtr); //move on to the position of the null character
    //*buffPtr = ' '; //replace with a space
    //buffPtr++; //move on ready for next
  }
  //buffPtr--; //move back a character to where the final space (' ') is
  *buffPtr = '\0'; //replace it with a null to terminate the string
  char dest[sizeof(buffPtr)+8];
  if(switchNumber == 2)
    strcpy(dest,"'R");
  else
    strcpy(dest,"'Q");
  strcat(dest,leng);
  strcat(dest,rowHex);
  strcat(dest,colHex);
  strcat(dest,buffer);
  Serial.write(dest);

  delay(3);
  
  return Serial.read() == 0x61;
}

/*
 * Inputs: switchNumber: 8-bit integer from 1-2 representing the desired switch (1 for left, 2 for right)
 * Outputs: 1 if SmartSwitch acknowledges command, 0 if SmartSwitch does not acknowledge command
 * Function: Displays the next image from flash memory to the selected switch
 */
int SS_display_next_image(uint8_t switchNumber){
  uint8_t switchCommand = 0;
  if(switchNumber == 2)
    switchCommand = 0x34;
  else
    switchCommand = 0x33;
  uint8_t msg[] = {0x2E, switchCommand}; 
  Serial.write(msg, sizeof(msg));

  delay(3);
  
  return Serial.read() == 0x61;
}

/*
 * Inputs: switchNumber: 8-bit integer from 1-2 representing the desired switch (1 for left, 2 for right)
 * Outputs: 1 if SmartSwitch acknowledges command, 0 if SmartSwitch does not acknowledge command
 * Function: Displays the previous image from flash memory to the selected switch
 */
int SS_display_previous_image(uint8_t switchNumber){
  uint8_t switchCommand = 0;
  if(switchNumber == 2)
    switchCommand = 0x36;
  else
    switchCommand = 0x35;
  uint8_t msg[] = {0x2E, switchCommand}; 
  Serial.write(msg, sizeof(msg));

  delay(3);
  
  return Serial.read() == 0x61;
}

/*
 * Inputs: switchNumber: 8-bit integer from 1-2 representing the desired switch (1 for left, 2 for right)
 *         address: 16-bit integer representing the address of the image to load
 * Outputs: 1 if SmartSwitch acknowledges command, 0 if SmartSwitch does not acknowledge command
 * Function: Displays a specific image from flash memory to the selected switch loaded from the specified address
 */
int SS_display_image_from_memory_address(uint8_t switchNumber, uint16_t address){
  uint8_t switchCommand = 0;
  if(switchNumber == 2)
    switchCommand = 0x32;
  else
    switchCommand = 0x31;
  int addressAsciiHex1 = nibble_to_ascii_hex((address & 0xF000) >> 12);
  int addressAsciiHex2 = nibble_to_ascii_hex((address & 0x0F00) >> 8);
  int addressAsciiHex3 = nibble_to_ascii_hex((address & 0x00F0) >> 4);
  int addressAsciiHex4 = nibble_to_ascii_hex(address & 0x000F);
  uint8_t msg[] = {0x2E, switchCommand, addressAsciiHex1, addressAsciiHex2, addressAsciiHex3, addressAsciiHex4}; 
  Serial.write(msg, sizeof(msg));

  delay(3);
  
  return Serial.read() == 0x61;
}

/*
 * Inputs: None
 * Outputs: 1 if data was read, 0 if no data was read
 * Function: If data was read, that data is sent to the Serial monitor
 */
int SS_read_byte(){
  int incomingByte = 0;
  if (Serial.available() > 0) {
              // read the incoming byte:
              incomingByte = Serial.read();

              // say what you got:
              Serial.print("I received: ");
              Serial.println(incomingByte, HEX);
              return 1;
      }
  return 0;
}

/*
 * Inputs: None
 * Outputs: None
 * Function: Changes the values of global variables based on whether buttons have been pressed or released
 * Global Variables to define: left_button_released, left_button_pressed, right_button_pressed, right_button_released
 */
void SS_check_press_status(){
  int incomingByte = 0;
  while (Serial.available() > 0) {
              // read the incoming byte:
              incomingByte = Serial.read();

              Serial.print("I received: ");
              Serial.println(incomingByte, HEX);
              if(incomingByte == 0x81)
                left_button_pressed = 1;
              else if(incomingByte == 0x82)
                right_button_pressed = 1;
              else if(incomingByte == 0xB1)
                left_button_released = 1;
              else if(incomingByte == 0xB2)
                left_button_released = 1;
   }
}

/*
 * Inputs: None
 * Outputs: None
 * Function: Clears the values of global variables
 * Global Variables to define: left_button_released, left_button_pressed, right_button_pressed, right_button_released
 */
void SS_clear_button_variables(){
  left_button_pressed = 0;
  right_button_pressed = 0;
  left_button_released = 0;
  right_button_released = 0;
}

/*
 * Inputs: 8-bit integer representing a 4-bit "nibble"
 * Outputs: 1 if SmartSwitch acknowledges command, 0 if SmartSwitch does not acknowledge command
 * Function: Changes the color of text characters to the onColor and the background of those characters to the offColor
 */
int nibble_to_ascii_hex(uint8_t nibble){
  if (nibble < 0xA) // If the ASCII value is 0-9
  {
        return (int)(nibble + 0x30);
  }
  else if (nibble >= 0xA) // Or if the ASCII value is A-F
  {
        return (int)(nibble + 0x37);
  }     
  return 0;
}


/*
Images:
1 - Blank
2 - Green
3 - Red

State:
0 -   Idle
1..3- Reserving
4 -   Confirm
5 -   Success
*/
int state = 0;
int draw = 1; //Need to redraw?
int serialread = 0;

int buzzerPin = 4;
int ledPin = 7;
int pirPin = 2;

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);

  SPI.begin(); //RFID
  mfrc522.PCD_Init();
  
  Serial.begin(115200);
  SS_set_onoff_color(0x0000, 0xFFFF);
  SS_set_line_color(0x0000);
}

void loop()
{
  serialread = Serial.read();

  if (digitalRead(pirPin) == HIGH) {
    digitalWrite(ledPin, HIGH);
  } else {
    //digitalWrite(ledPin, LOW);
  }

  
  if(serialread != 0xFFFFFFFF)
  {
    //Serial.print("Serial: ");
    //Serial.println(serialread, HEX);
  }
  //Serial.println(serialread, HEX);
  if (state==0) //Idle
  {
    if(draw==1)
    {
        DHT.read11(A1); //Read temp and humidity
        SS_clear(1);
        SS_clear(2);
        SS_display_image_from_memory_address(1,3);
        SS_display_image_from_memory_address(2,1);
        SS_add_text("06",15,25,"7:00pm",1);
        SS_add_text("06",20,25,String(DHT.temperature)+"C",2);
        SS_add_text("06",35,25,String(DHT.humidity)+"%",2);
        SS_set_onoff_color(0x321E, 0xFFFF);
        SS_add_text("08",35,18,"RESERVED",1);
        SS_set_onoff_color(0x0000, 0xFFFF);
        draw = 0;
    }
    if(serialread == 0x81) //Left Button
    {
      state = 1;
      draw = 1;
      
      return;
    }
  }
  
  if (state==1) //Reserving
  {
    if(draw==1)
    {
        SS_clear(1);
        SS_clear(2);
        SS_set_onoff_color(0x321E, 0xFFFF);
        SS_add_text("0a",10,10,"> 7:00pm <",1);
        SS_set_onoff_color(0x0000, 0xFFFF);
        SS_add_text("06",20,10,"8:00pm",1);
        SS_add_text("06",30,10,"9:00pm",1);
        SS_add_text("0b",20,3,"Reservation",2);
        SS_add_text("05",35,26,"Taken",2);
        draw = 0;
    }
    if(serialread == 0x81) //Left Button
    {
      state=2;
      draw = 1;
      return;
    }
    if(serialread == 0x82) //Right Button
    {
      //state=5;
      //draw = 1;
      return;
    }
  }
  
  
  if (state==2) //Reserving2
  {
    if(draw==1)
    {
        SS_clear(1);
        SS_clear(2);
        SS_add_text("06",10,10,"7:00pm",1);
        SS_set_onoff_color(0x5569, 0xFFFF);
        SS_add_text("0a",20,10,"> 8:00pm <",1);
        SS_set_onoff_color(0x0000, 0xFFFF);
        SS_add_text("06",30,10,"9:00pm",1);
        SS_add_text("09",20,11,"Scan Card",2);
        SS_add_text("0a",35,9,"To Reserve",2);
        draw = 0;
    }
    if(serialread == 0x81) //Left Button
    {
      state=3;
      draw = 1;
      return;
    }
    if (!mfrc522.PICC_IsNewCardPresent()) 
    {
      return;
    }
    state=5;
    draw=1;
    if(serialread == 0x82) //Right Button
    {
      state=5;
      draw=1;
      return;
    }
  }
  
  
  if (state==3) //Reserving3
  {
    if(draw==1)
    {
        SS_clear(1);
        SS_clear(2);
        SS_add_text("06",10,10,"7:00pm",1);
        SS_add_text("06",20,10,"8:00pm",1);
        SS_set_onoff_color(0x5569, 0xFFFF);
        SS_add_text("0a",30,10,"> 9:00pm <",1);
        SS_set_onoff_color(0x0000, 0xFFFF);
        SS_add_text("09",20,11,"Scan Card",2);
        SS_add_text("0a",35,9,"To Reserve",2);
        draw = 0;
    }
    if(serialread == 0x81) //Left Button
    {
      state=1;
      draw = 1;
      return;
    }
    if (!mfrc522.PICC_IsNewCardPresent()) 
    {
      return;
    }
    state=5;
    draw=1;
    if(serialread == 0x82) //Right Button
    {
      state=5;
      draw=1;
      return;
    }
  }
  
  
  if (state==5) //Success
  {
    if(draw==1)
    {
        tone(buzzerPin, 3000);
        delay(500);
        tone(buzzerPin, 4000);
        delay(500);
        noTone(buzzerPin);
        SS_clear(1);
        SS_clear(2);
        SS_add_text("05",13,10,"Name:",1);
        SS_add_text("0a",27,10,"Daniel Kim",1);
        SS_add_text("0a",40,10,"1214660696",1);
        SS_add_text("0b",20,3,"Reservation",2);
        SS_add_text("0a",35,7,"Successful",2);
        draw = 0;
    }
    if(serialread == 0x81) //Left Button
    {
      state=0;
      draw=1;
      return;
    }
    if(serialread == 0x82) //Left Button
    {
      state=0;
      draw=1;
      return;
    }
  }
  
}
