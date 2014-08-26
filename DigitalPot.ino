/***************************************************************************
 * Mega Mini Synth
 * 
 * An Arduino Mega2560 based music synthesizer
 * 
 * See http://megaminisynth.blogspot.co.nz/ for details.
 * 
 * This file controls the digital pots
 * 
 * 
 * D. Quentin McDonald
 * dqmcdonald@gmail.com
 * 2014
 * 
 ****************************************************************************/


#include <SPI.h>

DigitalPot::DigitalPot( int ss_pin, int num_channels )
{
  m_ss_pin = ss_pin;
  m_num_channels = num_channels;
  // set the slaveSelectPin as an output:
  pinMode (m_ss_pin, OUTPUT);
  // initialize SPI:
  SPI.begin(); 
}

void DigitalPot::setValue( int channel, int val )
{
  if( channel < 0 || channel >= m_num_channels ) {
    return;
  }

  if( val < 0 || val > 255 ) {
    return; 
  }

  // take the SS pin low to select the chip:
  digitalWrite(m_ss_pin,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(channel);
  SPI.transfer(val);
  // take the SS pin high to de-select the chip:
  digitalWrite(m_ss_pin,HIGH); 

}



