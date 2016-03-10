This directory contains files to put onto an SD card to be inserted in the Kids Player project.

.snd files contain the sound effects. The files contain unsigned 8 bit mono audio sampled at 44.1 kHz. Also, the 
originals may have been cut to shorten their duration.
This project uses sounds from freesound:
good.snd: http://www.freesound.org/people/joedeshon/sounds/119025/ 
wrong.snd: http://www.freesound.org/people/fisch12345/sounds/325113/
red.snd: http://freesound.org/people/margo_heston/sounds/196544/
green.snd: http://freesound.org/people/margo_heston/sounds/196521/
blue.snd: http://freesound.org/people/margo_heston/sounds/196535/
purple.snd: http://freesound.org/people/margo_heston/sounds/196547/
yellow.snd: http://freesound.org/people/margo_heston/sounds/196531/

.bmp files are made with the 'paint' program that comes with windows 10. They are in 1-bit (black/white) format 
and have the same dimensions as the LCD used: 128x64 pixels.

.scr files define a screen. They contain a json object.
The root object may have up to 5 entries:
"Init": This structure is executed immediately after loading the .scr file.
"B1": This structure is executed when the 1st (leftmost) button is pressed.
"B2": This structure is executed when the 2st button is pressed.
"B3": This structure is executed when the 3st button is pressed.
"B4": This structure is executed when the 4st (rightmost) button is pressed.
Each entry may define the following fields:
"image" (string): Specifies a .bmp file to put on the LCD.
"color" (string): Specifies a color for the LCD background light LED. The format is "0xRRGGBB".
Each of the colors red, green and blue is specified by two hexadecimal digits.
"screen" (string): Specifies a .scr file to load and display after processing of the entry is completed.
"sound" (string): Specifies a .snd file to be played.

The .bmp and .scr files are in the public domain.
