
#include "I2C_parser.h"


I2CParser BH1750;

void setup() {
  // put your setup code here, to run once:

  Serial.begin( 57600 );
  delay(1000);

  BH1750.set_addr( 0x77 );
  BH1750.set_exec( "S,W16,E" );
  BH1750.set_exec( "S,F2,R,R,E" );
  BH1750.set_pars( "0,1" );
  BH1750.set_calc( "$,1.2,/" );
  Wire.begin();


  Serial.println( "***************" );
  BH1750.show_init();
  Serial.println( "***************" );
  BH1750.show_exec();
  Serial.println( "***************" );
  BH1750.show_pars();
  Serial.println( "***************" );
  BH1750.show_calc();
  Serial.println( "***************" );

  BH1750._init();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  BH1750._exec();

  // read second from BH1750  
  Serial.print( "Get data:" );
  Serial.println( ( long ) BH1750.get_data(), BIN );
  Serial.print( "Get value:" );
  Serial.println( BH1750.get_value() );

  delay(1000);

}
