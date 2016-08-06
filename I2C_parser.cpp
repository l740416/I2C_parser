/*

I2CParser.h - Arduino library for string defined I2C communications

*/

#include "Arduino.h"
#include "I2C_parser.h"

I2CParser::I2CParser() {
  init_cmds_n = 0;
  exec_cmds_n = 0;
  pars_cmds_n = 0;
  calc_cmds_n = 0;   
  raw_data_n  = 0;
}

I2CParser::I2CParser( uint8_t address, String init, String exec, String pars, String calc ) {
  init_cmds_n = 0;
  exec_cmds_n = 0;
  pars_cmds_n = 0;
  calc_cmds_n = 0;   
  raw_data_n  = 0;
  set_addr( address );
  set_init( init );
  set_exec( exec );
  set_pars( pars );
  set_calc( calc );
}


int I2CParser::get_next_num( String &str, int &idx ) {
  int start = idx;
  while( isdigit( str[ idx ]) ) { idx++; }
  return str.substring( start, idx ).toInt();
}

byte I2CParser::pasre_cmds( String str, struct comm *cmd, byte &cmd_n ) {

  /*
    S: beginTransmission
    E: endTransmission
    W: write
    R: read
    F: requestFrom
    D: delay
  */
  
  str += ",";
  cmd_n = 0;
  int v = 0;
  
  for( int i = 0; i < str.length(); i++ ) {
    
    switch( str[i++] ) {
      
      case 'S':
        cmd[ cmd_n ].func = &I2CParser::do_beginTransmission;
        cmd[ cmd_n ].input = 0;
        break; 

      case 'E':
        cmd[ cmd_n ].func = &I2CParser::do_endTransmission;
        cmd[ cmd_n ].input = 0;
        break; 

      case 'R':
        cmd[ cmd_n ].func = &I2CParser::do_read;
        cmd[ cmd_n ].input = 0;
        break; 

      case 'W':
        v = get_next_num( str, i );
        if( v < 0 || v > 255 ) return EXCEED_VAL_RANGE;          
        cmd[ cmd_n ].func = &I2CParser::do_write;
        cmd[ cmd_n ].input = (byte) v;
        break; 
        
      case 'F':
        v = get_next_num( str, i );
        if( v < 0 || v > 255 ) return EXCEED_VAL_RANGE;          
        cmd[ cmd_n ].func = &I2CParser::do_requestFrom;
        cmd[ cmd_n ].input = (byte) v;
        break; 

      case 'D':
        v = get_next_num( str, i );
        if( v < 0 || v > 255 ) return EXCEED_VAL_RANGE;          
        cmd[ cmd_n ].func = &I2CParser::do_delay;
        cmd[ cmd_n ].input = (byte) v;
        break; 
     
       default:
        return SYNTAX_ERROR;   
        
    }
    
    if( str[i] != ',' ) return SYNTAX_ERROR;   
    cmd_n += 1;
    if( cmd_n >= MAX_CMD_LENGTH ) return EXCEED_MAX_LENGTH;
  }
  
  return NO_ERROR;
}
  
byte I2CParser::set_init( String init ) {
  return pasre_cmds( init, init_cmds, init_cmds_n );
}

byte I2CParser::set_exec( String exec ) {
  return pasre_cmds( exec, exec_cmds, exec_cmds_n );
}

byte I2CParser::set_pars( String pars ) {
  
  pars += ",";
  pars_cmds_n = 0;
  int v1 = 0;
  int v2 = 8;
  
  for( int i = 0; i < pars.length(); i++ ) {
    
    v1 = get_next_num( pars, i );
    if( pars[i] == ':' ) { 
      i += 1; 
      v2 = get_next_num( pars, i ); 
      if( v2 >= 8 ) return EXCEED_MAX_LENGTH;
    }

    if( pars[i] != ',' ) return SYNTAX_ERROR;
      
    if( v1 >= MAX_READ_LENGTH ) return EXCEED_MAX_LENGTH;

    pars_cmds[ pars_cmds_n ].byte_idx = v1;
    pars_cmds[ pars_cmds_n ].bit_idx  = v2;
    pars_cmds_n += 1;
    if( pars_cmds_n >= MAX_PARS_LENGTH ) return EXCEED_MAX_LENGTH;
  }
  return NO_ERROR;
}

byte I2CParser::set_calc( String calc ) {
  
  /*
    $ is the raw data generated after pars phase and all values shoud be non-negative number.
    the equation must represented as prefix order
    e.g -45 + 175 * $ / 99 should be  $,99,/,175,*,45,-
  */
  
  /* if no calc, pass raw data to value */
  if( calc == "" ) calc ="$";
    
  calc += ",";
  calc_cmds_n = 0;
  int v = 0;
  
  for( int i = 0; i < calc.length(); i++ ) {
    
    switch( calc[i++] ) {
      
      case '$':
        calc_cmds[ calc_cmds_n ].func       = NULL;
        calc_cmds[ calc_cmds_n ].value      = &data;
        calc_cmds[ calc_cmds_n ].phy_value  = v;
        break;
        
      case '+': 
        calc_cmds[ calc_cmds_n ].func       = &I2CParser::do_add;
        calc_cmds[ calc_cmds_n ].value      = &( calc_cmds[ calc_cmds_n ].phy_value );
        calc_cmds[ calc_cmds_n ].phy_value  = v;
        break;
        
      case '-': 
        calc_cmds[ calc_cmds_n ].func       = &I2CParser::do_sub;
        calc_cmds[ calc_cmds_n ].value      = &( calc_cmds[ calc_cmds_n ].phy_value );
        calc_cmds[ calc_cmds_n ].phy_value  = v;
        break;
        
      case '*': 
        calc_cmds[ calc_cmds_n ].func       = &I2CParser::do_mul;
        calc_cmds[ calc_cmds_n ].value      = &( calc_cmds[ calc_cmds_n ].phy_value );
        calc_cmds[ calc_cmds_n ].phy_value  = v;
        break;
        
      case '/': 
        calc_cmds[ calc_cmds_n ].func       = &I2CParser::do_div;
        calc_cmds[ calc_cmds_n ].value      = &( calc_cmds[ calc_cmds_n ].phy_value );
        calc_cmds[ calc_cmds_n ].phy_value  = v;
        break;
        
      case '>': 
        calc_cmds[ calc_cmds_n ].func       = &I2CParser::do_rsh;
        calc_cmds[ calc_cmds_n ].value      = &( calc_cmds[ calc_cmds_n ].phy_value );
        calc_cmds[ calc_cmds_n ].phy_value  = v;
        break;
        
      case '<': 
        calc_cmds[ calc_cmds_n ].func       = &I2CParser::do_lsh;
        calc_cmds[ calc_cmds_n ].value      = &( calc_cmds[ calc_cmds_n ].phy_value );
        calc_cmds[ calc_cmds_n ].phy_value  = v;
        break;
     
      default:
        i -= 1;
        v = get_next_num( calc, i );
        calc_cmds[ calc_cmds_n ].func       = NULL;
        calc_cmds[ calc_cmds_n ].value      = &( calc_cmds[ calc_cmds_n ].phy_value );
        calc_cmds[ calc_cmds_n ].phy_value  = v;
    }

    if( calc[i] != ',' ) return SYNTAX_ERROR;   
    calc_cmds_n += 1;
    if( calc_cmds_n >= MAX_CALC_LENGTH ) return EXCEED_MAX_LENGTH;
  }
  
  /* 
    prefix-tree check
  */
  
  int checker = 0;
  for( int i = 0; i < calc_cmds_n; i++ ) {  
    if( calc_cmds[i].func == NULL ) checker += 1;
    else checker -= 1;
  }
  if( checker != 1 ) return SYNTAX_ERROR;    
    
  return NO_ERROR;
}

void I2CParser::_init() {
  for( int i = 0; i < init_cmds_n; i++ ) {
    (this->*init_cmds[i].func)( init_cmds[i].input );
  }
}

void I2CParser::_exec() {
  
  raw_data_n = 0;
  for( int i = 0; i < exec_cmds_n; i++ ) {
    (this->*exec_cmds[i].func)( exec_cmds[i].input );
  }
  
  long val = 0;
  for( int i = 0; i < pars_cmds_n; i++ ) {      
    byte v = get_bits( pars_cmds[i].byte_idx, pars_cmds[i].bit_idx );
    if( pars_cmds[i].bit_idx > 7 ) val = ( val << 8 ) | v;
    else val = ( val << 1 ) | v;
  }
  data = ( float ) val;
  
  float stack_v[ MAX_CALC_LENGTH ];
  int stack_ptr = 0;
  
  for( int i = 0; i < calc_cmds_n; i++ ) {  
    
    if( calc_cmds[i].func == NULL ) {
      stack_v[ stack_ptr++ ] = *(calc_cmds[i].value);

    } else {
      float val2 = stack_v[ --stack_ptr ];
      float val1 = stack_v[ --stack_ptr ];
      stack_v[ stack_ptr++ ] = (this->*calc_cmds[i].func)( val1, val2 );
    }
  }
  value = stack_v[0];
  
  
  
  /*
  value = *(calc_cmds[0].value);
  for( int i = 1; i < calc_cmds_n; i += 2 ) {  
    value = (this->*calc_cmds[i+1].func)( value, *(calc_cmds[i].value) );
  }
*/
}



void I2CParser::show_cmds( struct comm *cmd, byte cmd_n ) {
  
  for( int i = 0; i < cmd_n; i++ ) {
    
    if( cmd[i].func == &I2CParser::do_beginTransmission ) {
      Serial.print( "beginTransmission(" );
      Serial.print( address, HEX );
      Serial.println( ")" );
      
    } else if( cmd[i].func == &I2CParser::do_endTransmission ) {
      Serial.println( "endTransmission()" );
      
    } else if( cmd[i].func == &I2CParser::do_write ) {
      Serial.print( "write(" );
      Serial.print( cmd[i].input );
      Serial.println( ")" );
      
    } else if( cmd[i].func == &I2CParser::do_read ) {
      Serial.println( "read()" );
      
    } else if( cmd[i].func == &I2CParser::do_requestFrom ) {
      Serial.print( "requestFrom(" );
      Serial.print( address, HEX );
      Serial.print( "," );
      Serial.print( cmd[i].input );
      Serial.println( ")" );
      
    } else if( cmd[i].func == &I2CParser::do_delay ) {
      Serial.print( "delay(" );
      Serial.print( DELAY_UNIT );
      Serial.println( ")" );
      
    }
  }
}

void I2CParser::show_pars() { 
  
  for( int i = 0; i < pars_cmds_n; i++ ) {
    
    if( pars_cmds[i].bit_idx == 8 ) {
      Serial.print( "B" );
      Serial.print( pars_cmds[i].byte_idx );
    } else {
      Serial.print( "B" );
      Serial.print( pars_cmds[i].byte_idx );
      Serial.print( ":" );
      Serial.print( pars_cmds[i].bit_idx );
    }
    
    if( i == pars_cmds_n - 1 )Serial.println( "" );
    else Serial.print( " + " );
  };
}

void I2CParser::show_calc() { 
  
  

  for( int i = 0; i < calc_cmds_n; i++ ) { 
     
    
    if( calc_cmds[i].func == NULL ) {
      
      if( calc_cmds[i].value == &data ) Serial.print( "$" );
      else Serial.print( calc_cmds[i].phy_value );

    } else {

      if( calc_cmds[i].func == &I2CParser::do_add ) Serial.print( "+" );
      else if( calc_cmds[i].func == &I2CParser::do_sub ) Serial.print( "-" );
      else if( calc_cmds[i].func == &I2CParser::do_mul ) Serial.print( "*" );
      else if( calc_cmds[i].func == &I2CParser::do_div ) Serial.print( "/" );
      else if( calc_cmds[i].func == &I2CParser::do_lsh ) Serial.print( "<<" );
      else if( calc_cmds[i].func == &I2CParser::do_rsh ) Serial.print( ">>" );

    }
    
    Serial.print( ", " );
  }
  
  Serial.println( "" );
}

