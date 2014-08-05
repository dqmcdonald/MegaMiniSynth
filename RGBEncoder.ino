/***************************************************************************
 * Mega Mini Synth
 * 
 * An Arduino Mega2560 based music synthesizer
 * 
 * See http://megaminisynth.blogspot.co.nz/ for details.
 * 
 * This file contains the code for the rotary encoder handling. This is an RGB illuminated
 * rotary encoder as described at
 * https://www.sparkfun.com/products/10982
 * 
 * This is based on the // This class subclasses the Encoder library described here:
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 * As such the pins the encoder pins should be attached to hardware interrupts.
 * Additions to this class are support for the RGB led and the push button.
 *
 * D. Quentin McDonald
 * dqmcdonald@gmail.com
 * 2014
 * 
 ****************************************************************************/
#include <Encoder.h>


#define BUTTON_DOWN HIGH
#define BUTTON_DEBOUNCE_TIME 10  // Switch has to be held for 10ms to be considered down
#define BUTTON_CHECK_PERIOD 20   // Check button state every 20 ms.



RGBEncoder::RGBEncoder( int enc_pin_1, int enc_pin_2, int red_pin, int blue_pin, 
int green_pin, int button_pin ): 
Encoder( enc_pin_1, enc_pin_2 ) {


  m_red_pin  = red_pin;
  m_green_pin = green_pin;
  m_blue_pin = blue_pin;
  m_button_pin = button_pin;
  m_button_pressed = false;

}


// Start the rotary encoder operating
void RGBEncoder::begin() {

  // Set RGB Pin Modes for Output
  pinMode( m_red_pin, OUTPUT );
  pinMode( m_green_pin, OUTPUT );
  pinMode( m_blue_pin, OUTPUT );

  // Set Switch mode for input:
  pinMode( m_button_pin, INPUT );

  // Initialize the encoder value:
  write(0);

  m_button_down_time = -1;
  m_button_check_time = millis() + BUTTON_CHECK_PERIOD;

}

// Set the color based on the RGB values. 
// Each one is in the range 0-255;
void RGBEncoder::setColor( int red, int green, int blue ) {

  int r,g,b;
  r=red;
  if( r < 0 )
    r = 0;
  if( r > 255 )
    r = 255;

  g=green;
  if( g < 0 )
    g = 0;
  if( g > 255 )
    g = 255;

  b=blue;
  if( b < 0 )
    b = 0;
  if( b > 255 )
    b = 255;

 

  //Write the values to the digital pins
  analogWrite( m_red_pin, 255-r );
  analogWrite( m_green_pin, 255-g );
  analogWrite( m_blue_pin, 255-b );


} 

// Check for button press. This should be called as frequently as possible.
void RGBEncoder::update ( )
{

  // If we already know the button has been pressed then there's nothing to do here:
  if( m_button_pressed )
    return;

  // We only check the button every 20 ms or so. If we aren't ready to do that then
  // just return now:
  if( millis() < m_button_check_time ) 
    return;

  // Reset the button check time
  m_button_check_time = millis() + BUTTON_CHECK_PERIOD;


  // Check if the pin is in the button down mode:
  if( digitalRead( m_button_pin ) == BUTTON_DOWN ) {
    // For debouncing don't decide right away. See if we are timing how long it is
    // down for:
    if( m_button_down_time < 0 ) {
      // We aren't timing. So start now. Check again in 10ms:
      m_button_down_time = millis() + BUTTON_DEBOUNCE_TIME;

    } 
    else {
      // We have been timing the button. See if we've waited long enough:
      if( millis() >= m_button_down_time ) {
        m_button_pressed = true;
        m_button_down_time = -1;
      }

    }

  }



}


boolean RGBEncoder::buttonPressed() {

  boolean pressed = m_button_pressed;

  m_button_pressed = false;

  return pressed;


}












