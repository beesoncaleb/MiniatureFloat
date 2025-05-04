# Minifloat
Team Name: Minisink

Members:
- Caleb Beeson
- Christina Botello
- Onyeze Chineke

## Description
This contains all code related to the function of our team's miniature float device, including code uploaded to our microcontroller and code necessary for communication and operation of the float.

## How it works
The esp32 microcontroller when powered will begin a wifi network as well as a web server. From here I can connect a computer to the wifi network and run the minisink.py program. This program communicates to the esp32 via http requests to the web server. The program will send command via http and listen for responses and handle them appropriately. The commands defined by the code are:
- PROFILE1 - Begin the first depth profile to a depth of 9 feet.
- PROFILE2 - Begin the next depth profile to a depth of 4m resurfaces then dives to a depth of 2m.
- RESET - A command that can end any depth profile prematurely and reset the esp32 microcontroller.

Additional commands for minisink.py that are not communicated to the esp32.
- PLOT - Visualizes data obtained from the most recent depth profile.
- EXIT - End the program execution.

All depth profiles communicate a csv file of the data obtained from the profile: temperature, depth, pressure. These csv files can be plotted using the plot command, it returns a graph of the data plotted over time.

## Dependencies
The minisink sketch requires the following libraries
- WiFi
- WebServer
- Stepper
- Wire
- MS5837

The minisink python program requires these libraries
- matplotlib
- pandas
- pytz
