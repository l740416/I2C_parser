

#include "I2C_parser.h"

I2CParser DS3231;

void setup() {
  // put your setup code here, to run once:

  Serial.begin( 57600 );
  delay(1000);

  DS3231.set_addr( 104 );
  DS3231.set_exec( "S,W0,E,F7,R,R,R,R,R,R,R" );
  DS3231.set_pars( "0:6,0:5,0:4,0:3,0:2,0:1,0:0" );
  DS3231.set_calc( "$,6,$,4,>,*,-" );
  Wire.begin();


  Serial.println( "***************" );
  DS3231.show_init();
  Serial.println( "***************" );
  DS3231.show_exec();
  Serial.println( "***************" );
  DS3231.show_pars();
  Serial.println( "***************" );
  DS3231.show_calc();
  Serial.println( "***************" );

  DS3231._init();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  DS3231._exec();

  // read second from DS3231  
  Serial.print( "Get data:" );
  Serial.println( ( long ) DS3231.get_data(), BIN );
  Serial.print( "Get value:" );
  Serial.println( DS3231.get_value() );

  delay(1000);

}
