#include "DHT.h" //using https://github.com/Seeed-Studio/Grove_Temperature_And_Humidity_Sensor

#define DHTPIN 2     // what pin we're connected to
//pins for driving the meters using analogwrite. Not all pins can do this, see https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/
#define HUMPIN 11 //d11 pin for humidity meter = OCR2A
#define TEMPPINPLUS 3 //d3 pin for temperature meter = OCR2B
#define TEMPPINMIN 9 //Pin for other side of temperature meter (needed since it goes two ways), no need for pwm on this pin

// Uncomment whatever type sensor you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() 
{
    Serial.begin(9600); //Serial output for debugging 
    Serial.println("Debug output for temp humidity meter!");
    Wire.begin(); //Start Wire for dht
    dht.begin(); //start sensor

    //start high frequency pwm with some magic register operations:
    TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20); //non-inverting phase correct 8bit pwm, enable a/b
    TCCR2B = _BV(CS20); //no prescaler (30khz)*/
    OCR2A = 0; 
    
    pinMode(HUMPIN, OUTPUT); //set humidity pin as output, otherwise analogwrite won't work.
    pinMode(TEMPPINPLUS, OUTPUT); //same for temperature pin plus
    pinMode(TEMPPINMIN, OUTPUT); //same for temperature pin min

    //start up with some tuning and testing for both meters
    digitalWrite(TEMPPINMIN, LOW); //for temperatures above 15 celcius
    OCR2A = 50; //set humidity to 50 for testing & tuning 
    OCR2B = 50; //set temperature to pwmval 50 = 20 degrees for testing & tuning
    delay(2000);
    OCR2A = 100; //set humidty to 100 for testing & tuning
    OCR2B = 150; //set temperature to pwmval 150 = 30 degrees for testing & tuning
    delay(2000);
    OCR2A = 0; //reset to zero
    OCR2B = 0; //reset to zero

    //continue testing with temperature meter:
    digitalWrite(TEMPPINMIN, HIGH); //for temperatures below 15 celcius
    OCR2B = 205; //set temperature to pwmval 255-50 = 10 degrees for testing & tuning
    delay(2000);
    OCR2B = 105; //set temperature to pwmval 255-150 = 0 degrees for testing & tuning
    delay(2000);
    OCR2B = 255; //reset to zero
    
}

void loop() 
{
    float temp_hum_val[2] = {0};
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    if(!dht.readTempAndHumidity(temp_hum_val)){
        //write temperature to serial output
        Serial.print("Humidity: "); 
        Serial.print(temp_hum_val[0]);
        
        OCR2A = temp_hum_val[0]; //write humidity to meter (no calculations needed here, since we created a 1 to 1 relation!)
        
        Serial.print(" %\t");
        Serial.print("Temperature: "); 
        Serial.print(temp_hum_val[1]);
        Serial.println(" *C");
      
        if (temp_hum_val[1] > 30) { //we cannot display higher than 30
          temp_hum_val[1] = 30; //so set to 30 in this case
        }

        if (temp_hum_val[1] < 0) { //se cannot display lower than 30
          temp_hum_val[1] = 0; //so set to 0 in this case
        }

        if (temp_hum_val[1] >= 15) {//if temperature is higher than 15
          digitalWrite(TEMPPINMIN, LOW); //for temperatures above 15 celcius
          OCR2B = (temp_hum_val[1]-15)*10; //pwm value = 10 times the temperature minus 15
        }
        else { //for temperatures below 15
          digitalWrite(TEMPPINMIN, HIGH); //for temperatures above 15 celcius
          OCR2B = 255-(temp_hum_val[1]*10); //pwm value = 255 minus 10 times the temperature
        }
    }
    else{
       Serial.println("Failed to get temprature and humidity value.");
    }
    delay(1500); //wait a bit for the sensor and meters to settle in for the next measurement
}
