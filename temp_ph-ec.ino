int temp=0.0;                   //where the final temperature data is stored
double ec=0.0;                     // electrical conductivity
double ph=0.0;                     // pH
double tds=0.0;                    // Total dissolved solids

String inputstring = "";
String sensorstring = "";
bool input_string_complete = false;
bool sensor_string_complete = false;
bool pH_reading = false;

char computerdata[20];               //A 20 byte character array to hold incoming data from a pc/mac/other
char sensordata[30];                 //A 30 byte character array to hold incoming data from the sensors
byte computer_bytes_received=0;      //We need to know how many characters bytes have been received
byte sensor_bytes_received=0;        //We need to know how many characters bytes have been received

char ph_data[20]; //we make a 20 byte character array to hold incoming data from the pH.
byte received_from_sensor=0; //we need to know how many characters have been received.


int s0 = D7;             // pin D7 to control serial pin S0
int s1 = D6;             // pin D6 to control serial pin S1
int chEC = 0;            // channel 0 on the serial controler is wired to the EC sensor
int chpH = 1;            // channel 1 on the serial controler is wired to the pH sensor
int baudrateCh0 = 9600;
int baudrateCh1 = 9600;

//int sendpHCommand(String command);  // function for sending UART commands to pH sensor
//int sendECCommand(String command);  // function for sending UART commands to EC sensor

void setup()  // run once on power on.
{
  Particle.variable("Temp", &temp, INT);
  Particle.variable("pH", &ph, DOUBLE);
  Particle.variable("EC", &ec, DOUBLE);
  Particle.variable("TDS", &tds, DOUBLE);

  Serial.begin(38400);        // Initialize USB to computer

  pinMode(D7, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D2, OUTPUT);        // temp sensor
  pinMode(A0, INPUT_PULLUP);  // INPUT_PULLUP is a bit clearer than the
                              // Arduino way with digitalWrite() on an INPUT to
                              // attach/detach an internal pull-up resistor

  openChannel(chEC);
  Serial1.print("C,1\r");
  //Serial1.println("O,SG,0");  //disable the specific gravity
  //Serial1.println("O,S,0");   //disable the salinity.
  openChannel(chpH);
  Serial1.print("C,1\r");       // C,1<CR> Continuous mode enabled

//  Particle.function("phCommand", sendpHCommand); //register Particle cloud function
//  Particle.function("ecCommand", sendECCommand); //register Particle cloud function

  inputstring.reserve(10); //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30); //set aside some bytes for receiving data from Atlas Scientific product

}

void loop() {
  temp = getTemp();
  Serial.print("Temp: ");
  Serial.println(temp);       // print to serial
  delay(1000);

//  ec = getEC();
//  Serial.print("EC: ");
//  Serial.println(ec);
  delay(1000);
  ph = getpH();
  Serial.print("pH: ");
  Serial.println(ph);
  delay(1000);
}

float getTemp(void){          //the read temperature function
  float v_out;                  //voltage output from temp sensor in mV
  float temp;                   //the final temperature is stored here
  digitalWrite(D2, HIGH);       // power up the sensor (but only 3.3V on the Photon)
  delay(2);
  v_out = analogRead(A0);       // get the voltage reading
  digitalWrite(D2, LOW);        // power off again

  v_out *= (3.3 / 4095.0) * 100 ;     // V only in mV already
  temp= 0.0512 * v_out - 20.5128;
  return temp;
}

void openChannel(int channel) {
  switch (channel) {                 // Looking to see what channel to open
    case 0:                          // If channel==0 then we open channel 0
      Serial.println("EC Channel");
      digitalWrite(s0, LOW);         // S0 and S1 control what channel opens
      digitalWrite(s1, LOW);         // S0 and S1 control what channel opens
      Serial1.begin(baudrateCh0);	   // reset serial to baudrate defined for this channel
      break;                         // Exit switch case
    case 1:
      Serial.println("pH Channel");
      digitalWrite(s0, HIGH);
      digitalWrite(s1, LOW);
      Serial1.begin(baudrateCh1);
      break;
  }
}

double getEC(void) {
  char sensorstring_array[30];
  char *EC;
  char *TDS;
  char *SAL;
  char *GRAV;
  openChannel(chEC);
  if (input_string_complete) {
    Serial1.print(inputstring);
    Serial1.print('\r');
    inputstring = "";
    input_string_complete = false;
  }
  if (Serial1.available() > 0) {
    char inchar = (char)Serial1.read();
    sensorstring += inchar;
    if (inchar == '\r') {
      sensor_string_complete = true;
    }
  }
    if (sensor_string_complete == true) {
      if (isdigit(sensorstring[0]) == false) {
        Serial.println(sensorstring);
      }
      else {
          sensorstring.toCharArray(sensorstring_array, 30);
          EC = strtok(sensorstring_array, ",");
          TDS = strtok(NULL, ",");
          SAL = strtok(NULL, ",");
          GRAV = strtok(NULL, ",");
        }
      sensorstring = "";
      sensor_string_complete = false;
    }
  }
  return 
}

double getTDS(void) {
  char sensorstring_array[30];
  char *EC;
  char *TDS;
  double d_tds = 0.0;
  openChannel(chEC);
  sensorstring.toCharArray(sensorstring_array, 30);
  EC = strtok(sensorstring_array, ",");
  TDS = strtok(sensorstring_array, ",");
  d_tds = strtod(TDS, NULL);
  return d_tds;
}

double getpH(void) {
  openChannel(chpH);
  //  Serial1.print("status\r");
    if (Serial1.available() > 0) {
  //      Serial.println("pH available");
      char inchar = (char)Serial1.read();
      sensorstring += inchar;
      if (inchar == '\r') {
        sensor_string_complete = true;
      }
      if (sensor_string_complete== true) {
  //        Serial.println(sensorstring);
        if (isdigit(sensorstring[0])) {
          pH_reading = true;
        }
        if (pH_reading == true) {
          ph = sensorstring.toFloat();
      }
      pH_reading = false;
    }
  return (double)ph;
  }
}
