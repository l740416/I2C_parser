/*

I2CParser.h - Arduino library for string defined I2C communications

*/



#ifndef _I2CPARSER_H_
#define _I2CPARSER_H_

#include "Arduino.h"
#include "Wire.h"

#define DELAY_UNIT       1000  // ms
#define CMD_PER_SET      4

#define MAX_CMD_LENGTH   16
#define MAX_READ_LENGTH  32
#define MAX_PARS_LENGTH  16
#define MAX_CALC_LENGTH  32

/* 
  the maximum number of different reading from a sensor
  this also refer to number of different pars/calc rules for a sensor  
*/
#define MAX_DATA_NUM     2


class I2CParser
{
  public:
    I2CParser();
    I2CParser( uint8_t address, String init, String exec, String pars, String calc );
    void set_addr( uint8_t addr ) { address = addr; }
    byte set_init( String init );
    byte set_exec( String exec );
    byte set_pars( String pars, byte idx = 0 );
    byte set_calc( String calc, byte idx = 0 );
    
    uint8_t get_address() { return address; }
    
    void _init();
    void _exec();
     
    long get_data( byte idx = 0 )  { return data[idx]; }
    float get_value( byte idx = 0 ) { return value[idx]; }
     
    enum error_type {
      NO_ERROR,
      EXCEED_MAX_LENGTH,
      EXCEED_VAL_RANGE,
      SYNTAX_ERROR
    };
    
    void show_init() { show_cmds( init_cmds, init_cmds_n ); }
    void show_exec() { show_cmds( exec_cmds, exec_cmds_n ); }
    void show_pars( byte idx = 0 );
    void show_calc( byte idx = 0 );
    
    
  private:
    uint8_t address;
    
    const byte bit_mask[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
    
    //typedef byte  (*I2C_CMD) ( byte input );
    typedef byte (I2CParser::*I2C_CMD) ( byte input );
    typedef float (I2CParser::*CALC_OP) ( float v1, float v2 );
      
    struct comm {
      I2C_CMD func;
      byte    input;
    };
    
    struct pars {
      int  byte_idx;
      byte bit_idx;
    };
    
    struct calc {
      CALC_OP func;
      float *value;
      float phy_value;
    };
    
    byte do_beginTransmission(  byte input ) { Wire.beginTransmission(address); return 0; }
    byte do_endTransmission  (  byte input ) { return Wire.endTransmission(); }
    byte do_write            (  byte input ) { return Wire.write( input ); }
    byte do_read             (  byte input ) { raw_data[ raw_data_n ] = Wire.read(); return raw_data[ raw_data_n++ ]; }
    byte do_requestFrom      (  byte input ) { return Wire.requestFrom( address, input ); }
    byte do_delay            (  byte input ) { delay( DELAY_UNIT ); return 0; }
    
    float do_nul( float v1, float v2 ) { return 0; }
    float do_add( float v1, float v2 ) { return v1 + v2; }
    float do_sub( float v1, float v2 ) { return v1 - v2; }
    float do_mul( float v1, float v2 ) { return v1 * v2; }
    float do_div( float v1, float v2 ) { return v1 / v2; }
    float do_lsh( float v1, float v2 ) { return ( float ) ( ( (int) v1 ) << ( (int) v2 ) ); }
    float do_rsh( float v1, float v2 ) { return ( float ) ( ( (int) v1 ) >> ( (int) v2 ) ); }
    
    /*
      static struct pool, beta
    */
    struct comm_set {
      byte assigned;
      comm cmds[ CMD_PER_SET ];
      comm_set *next;
    };
    static comm_set comm_pool[ 20 ];

    struct pars_set {
      byte assigned;
      pars cmds[ CMD_PER_SET ];
      pars_set *next;
    };
    static pars_set pars_pool[ 20 ];
    
    struct calc_set {
      byte assigned;
      calc cmds[ CMD_PER_SET ];
      calc_set *next;
    };
    static calc_set calc_pool[ 20 ];
            
    byte init_cmds_n;
    byte exec_cmds_n;
    byte pars_cmds_n[ MAX_DATA_NUM ];
    byte calc_cmds_n[ MAX_DATA_NUM ];   
    struct comm init_cmds[ MAX_CMD_LENGTH ];
    struct comm exec_cmds[ MAX_CMD_LENGTH ]; 
    struct pars pars_cmds[ MAX_DATA_NUM ][ MAX_PARS_LENGTH ];
    struct calc calc_cmds[ MAX_DATA_NUM ][ MAX_CALC_LENGTH ];
    
    byte raw_data_n;
    byte raw_data[ MAX_READ_LENGTH ];
    
    float data[ MAX_DATA_NUM ];
    float value[ MAX_DATA_NUM ];
    
    byte get_bits( int idx1, byte idx2 ) { 
      if( idx2 > 7 ) return raw_data[ idx1 ];
      return ( raw_data[ idx1 ] & bit_mask[ idx2 ] ) != 0x00;
    }

    byte pasre_cmds( String str, struct comm *cmd, byte &cmd_n );
    int get_next_num( String &str, int &idx );
    
    void show_cmds( struct comm *cmd, byte cmd_n );
};

#endif