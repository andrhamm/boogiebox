# boogiebox

An electronic toy music player for toddlers. Let your toddler listen to their favorite songs whenever they want!

The user interface for the music player is comprised of 5 RGB NeoPixel LED arcade buttons which are dynamically controlled by the software. The software uses these buttons and their color state to produce an easy to understand menu system for use by toddlers and parents alike. To accompany the menu system, each song in the system gets a snazzy flash card containing an image related to the song and a color code to be entered in order to play the given song.

Example: the flash card for the song "Itsy Bitsy Spider" has an image of a spider and a "legend" for the color code _red, blue, green, green_. Press the Select Mode button followed by each of the corresponding buttons for the given colors (in the correct sequence) and the song starts playing!

---

# Overview

## Electronics

* 1x Adafruit M4 Express Feather
* 1x Adafruit Music Maker FeatherWing
* 2x 4 Ohm 3W speakers
* 5x 30mm transparent arcade buttons (from Adafruit)
* 1x Sheet of 5 Adafruit NeoPixel PCBs
* 1x microSD card, enough to fit 250+ MP3-format song files

## 3D Printing

Print 5 sets of diffusers for the arcade buttons as described in [this Adafruit tutorial](https://learn.adafruit.com/neopixel-arcade-button/3d-printing).

Work in progress: [the main enclosure](https://cad.onshape.com/documents/12da94011e69d1e3bd1c7b1f/w/2db13da0ebb3587c648491c8/e/66920aa1749b90669fdf5b23)

## Audio

Audio must be saved to the microSD card in MP3 format and follow the naming convention of `song1234.mp3` where `1234` is the song's unique 4-digit color code for the song. When adding new music, it is helpful to maintain an external file that links the color code / filename to the song. See [`songs.csv`](songs.csv).
