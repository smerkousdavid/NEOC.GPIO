#!/bin/bash

echo "Welcome to the quick and easy NEOC installer by David Smerkous"

echo "Double checking requirements"
sudo apt-get install build-essential doxygen git pkg-config 
echo "DONE!"

echo "Downloading repository..."
sudo git clone https://github.com/smerkousdavid/NEOC.GPIO

echo "
Entering directory"

cd NEOC.GPIO/NEOC

echo "
Compiling sources!"

sudo make

echo "
Installing sources!"
sudo make install

echo "
Cleaning sources!"
sudo make clean

echo "
Generating documentation!"

sudo ./gendocs.sh

echo "

Done Installing NEOC, compile and run an example like this 
\`\`\`
gcc fadeled.c -I/usr/include -lneo -o fadeled
sudo chmod 755 fadeled
./fadeled
\`\`\`
"

