/***************************************************************************
 * Mega Mini Synth
 * 
 * An Arduino Mega2560 based music synthesizer
 * 
 * See http://megaminisynth.blogspot.co.nz/ for details.
 * 
 * This file is the main routine for the synth - all command and control operations
 * are performed from here.
 * 
 * D. Quentin McDonald
 * dqmcdonald@gmail.com
 * 2014
 * 
 ****************************************************************************/

#include <MIDI.h>
#include <Encoder.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// Pin definitions:



#define ENC1_RED_PIN        2
#define ENC1_GREEN_PIN      3
#define ENC1_BUTTON_PIN     4
#define ENC1_BLUE_PIN       5

#define TIMER1_OUT         11
#define ENC1_PIN1          18
#define ENC1_PIN2          19






class Oscillator {

public:
  //
  Oscillator( int timer );        // Create an instance of an Oscillator using Timer "timer"
  void begin();
  void setNote( int current_note );     // Set the current MIDI note
  void setPitchBend( int pitch_bend ); // Set the Pitchbend amount
  void testScaling( int method );      // Simply a test of the pitch bend scaling

protected:
  void setPreScaling( int midi_note );// Set up the pre-scaler appropriate for the given note


    int m_note;
  int m_pitch_bend;
  int m_timer;
};


// This class subclasses the Encoder library described here:
// http://www.pjrc.com/teensy/td_libs_Encoder.html
// As such the pins the encoder pins should be attached to hardware interrupts.
// Additions to this class are support for the RGB led and the push button.
// The LED pins should support PWM
// The Button Pin needs a pull-down resistor on it and is normally low.
class RGBEncoder : 
public Encoder {

public:

  RGBEncoder( int enc_pin_1, int enc_pin_2, int red_pin, int blue_pin, int green_pin, int button_pin );

  void begin(); // Start the rotary encoder operating

  void setColor( int red, int green, int blue ); // Set the color based on the RGB values. 
  // Each one is in the range 2-255;

  void update ( );  // Read the current encoder value, looking for changes

  int getValue( );   // Get the current encoder value

    boolean buttonPressed(); // Returns true if the button has been pressed.


private:

  int m_red_pin;
  int m_green_pin;
  int m_blue_pin;
  int m_button_pin;
  boolean m_button_pressed;
  long int m_button_down_time;
  long int m_button_check_time;

};

// Object creation:


// Create Oscillator instances
Oscillator OSC1(1);

// Rotary Encoder instances:
RGBEncoder Encoder1( ENC1_PIN1, ENC1_PIN2, ENC1_RED_PIN, ENC1_GREEN_PIN, ENC1_BLUE_PIN,
ENC1_BUTTON_PIN );


void setup() {


  Serial.begin(9600);
  OSC1.begin();
  Encoder1.begin();
  Encoder1.setColor( 200,150, 0 );

  // Connect the callbacks for MIDI input - these will be called on certain MIDI events:
  // 
  MIDI.setHandleNoteOn(handleNoteOn);  
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandlePitchBend(handlePitchBend);

  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);


}





void loop() {

  // Call MIDI.read the fastest you can for real-time performance.
  MIDI.read();

  Encoder1.update();
  handleRotaryEncoders();


}


// ************************************************************************************
// MIDI Event handlers:


//A Note-on event has been received
void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  // Pass the note onto the oscillators
  OSC1.setNote(int(pitch));

}

// A Note-off event has been received
void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  // Do something when the note is released.
  // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.

}

// A pitch-bend event has been received:
void handlePitchBend( byte channel, int bend )
{
  // Pass the pitchbend onto the oscillators
  OSC1.setPitchBend(bend);

}


// ************************************************************************************
// Rotary Encoder Event Handlers

int last_val_enc1= 0;
int current_note = 69;

void handleRotaryEncoders( ) {

  if( Encoder1.buttonPressed()  ){
    // reset the current note to A
    current_note = 69;
    OSC1.setNote( current_note );
  }
  int val = Encoder1.read();
  if( val < last_val_enc1 ) {
    current_note--;
    if( current_note < 1 )
      current_note = 1;
    OSC1.setNote( current_note );
  } 
  else if( val > last_val_enc1 ) {
    current_note++;
    if( current_note > 126 ) {
      current_note = 126;
    }
    OSC1.setNote( current_note );
  }
  last_val_enc1 = val;
} 





















