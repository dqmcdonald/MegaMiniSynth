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
#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_ILI9340.h" // Hardware-specific library
#include <SPI.h>
#include <SD.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// Pin definitions:



#define ENC1_RED_PIN        4
#define ENC1_GREEN_PIN      6
#define ENC1_BUTTON_PIN     7
#define ENC1_BLUE_PIN       5

#define DIGITAL_POT_SS_PIN  9

#define TIMER1_OUT         11
#define ENC1_PIN1           2
#define ENC1_PIN2           3


#define TFT_RST 47
#define TFT_DC 49
#define TFT_CS 53
#define SD_CS 45

#define DIGITAL_SWITCH_PIN1 22
#define DIGITAL_SWITCH_PIN2 24


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


class DigitalPot {
public:
  DigitalPot( int ss_pin, int num_channels );

  // Value is from 0-255
  void setValue( int channel, int val );

protected:

  int m_ss_pin;
  int m_num_channels;



};


// Object creation:


// Create Oscillator instances
Oscillator OSC1(1);

// Rotary Encoder instances:
RGBEncoder Encoder1( ENC1_PIN1, ENC1_PIN2, ENC1_RED_PIN, ENC1_GREEN_PIN, ENC1_BLUE_PIN,
ENC1_BUTTON_PIN );

Adafruit_ILI9340 tft = Adafruit_ILI9340(TFT_CS, TFT_DC, TFT_RST);

// DigitalPot Instance:
DigitalPot pot1 = DigitalPot( DIGITAL_POT_SS_PIN, 2 );



int switch_mode;

void setup() {


  Serial.begin(9600);
  OSC1.begin();
  Encoder1.begin();
  Encoder1.setColor(255,0, 0 );
  delay(500);
  Encoder1.setColor( 0, 255, 0 );
  delay(500);
  Encoder1.setColor( 0, 0, 255 );
  delay(500);
  Encoder1.setColor( 255, 255, 0 );

  // Connect the callbacks for MIDI input - these will be called on certain MIDI events:
  // 
  MIDI.setHandleNoteOn(handleNoteOn);  
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandlePitchBend(handlePitchBend);

  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);


  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
    return;
  }
  Serial.println("OK!");

  tft.begin();
  tft.fillScreen(ILI9340_BLUE);

  tft.setRotation(1);
  bmpDraw("mms.bmp", 0, 0);
  delay(1000);
  tft.fillScreen(0xFFFF);
  drawSlider( 50, 100 );

  pinMode(DIGITAL_SWITCH_PIN1, OUTPUT );
  pinMode(DIGITAL_SWITCH_PIN2, OUTPUT );
  digitalWrite( DIGITAL_SWITCH_PIN1, LOW );
  digitalWrite( DIGITAL_SWITCH_PIN1, LOW );
  switch_mode = 0;

  pot1.setValue( 0, 127 );
  pot1.setValue( 1, 127 );
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
  static char* note_string[] = { 
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"   };
  int octave = (pitch / 12) - 1;
  int note_index = (pitch % 12);



  // Pass the note onto the oscillators
  OSC1.setNote(int(pitch));
  Serial.print("Note on ");
  Serial.println(pitch);
  setSwitchMode( switch_mode );

  tft.fillRect(40, 200, 80, 40, 0xFFFF);
  tft.setTextSize( 4 );
  tft.setTextColor( 0x0);
  tft.setCursor( 40,200);
  tft.print(note_string[note_index]);
  tft.print(octave);

}

// A Note-off event has been received
void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  // Do something when the note is released.
  // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.
  setSwitchMode( 0 );
  tft.fillRect(40, 200, 80, 40, 0xFFFF);
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
int current_pot_val = 127;

void handleRotaryEncoders( ) {

  if( Encoder1.buttonPressed()  ){
    // reset the current note to A
    //current_note = 69;
    // OSC1.setNote( current_note );
    switch_mode += 1;
    if( switch_mode == 4 ) {
      switch_mode =1;
    }

  }
  int val = Encoder1.read();
  if( val < last_val_enc1 ) {
    current_pot_val--;
    if( current_pot_val < 0 )
      current_pot_val = 0;

    drawSlider( current_pot_val, 255 );
    pot1.setValue( 0, current_pot_val );
  } 
  else if( val > last_val_enc1 ) {
    current_pot_val++;
    if( current_pot_val > 255 ) {
      current_note = 255;
    }

    drawSlider( current_pot_val, 255 );
    pot1.setValue( 0, current_pot_val );
  }

  last_val_enc1 = val;

} 

// Draw a slider rectangle to represent the current value:
void drawSlider( int val, int maxval ) {
  const int RECT_X = 40;
  const int RECT_Y = 20;
  const int RECT_WIDTH = 240;
  const int RECT_HEIGHT = 50;
  static int last_val = -1;
  if( last_val < 0 ) {
    tft.drawRect(RECT_X, RECT_Y, RECT_WIDTH, RECT_HEIGHT, 0x0);
    tft.setTextSize( 3 );
    tft.setTextColor( 0x0);
    tft.setCursor( RECT_X, RECT_Y+RECT_HEIGHT+5);
    tft.print("PWM");
  }
  // tft.fillRect( RECT_X + 1, RECT_Y+1, RECT_WIDTH-2, RECT_HEIGHT-2,
  //   0xFFFF );

  double frac = (double)val/maxval;
  int width = (int)( frac * (RECT_WIDTH-2));

  if( val >= last_val ) {
    tft.fillRect( RECT_X + 1, RECT_Y+1, width, RECT_HEIGHT-2,0x0 );
  } 
  else {
    tft.fillRect( RECT_X+width, RECT_Y+1, RECT_WIDTH-width-2, RECT_HEIGHT-2,
    0xFFFF );
  }

  last_val = val;
}

void setSwitchMode( int smode )
{
  if( smode == 0 ) {
    digitalWrite( DIGITAL_SWITCH_PIN1, LOW );
    digitalWrite( DIGITAL_SWITCH_PIN2, LOW );
  } 
  else if( smode == 1 ) {
    digitalWrite( DIGITAL_SWITCH_PIN1, HIGH );
    digitalWrite( DIGITAL_SWITCH_PIN2, LOW ); 
  } 
  else if (smode == 2 ) {
    digitalWrite( DIGITAL_SWITCH_PIN1, LOW );
    digitalWrite( DIGITAL_SWITCH_PIN2, HIGH ); 
  } 
  else if (smode == 3 ) {
    digitalWrite( DIGITAL_SWITCH_PIN1, HIGH );
    digitalWrite( DIGITAL_SWITCH_PIN2, HIGH );  
  }


}




// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint8_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); 
    Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); 
    Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); 
    Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); 
      Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
          pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File & f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File & f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
























