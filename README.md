# FIle Explanation
- main.c      File should be flashed onto STC32G
- main.py     File running on the computer side

# Principle Explanation

Assuming you alrady have a basic understanding of Keil, the main.c file is to be flashed onto the microcontroller, while the main.py file is for the computer side. Before running main.py, change the COM port in the program to the
one corresponding to your computer. After running, the computer will send information from the screen to the microcontroller via the serial port, and the microcontroller will directly forward it to the screen.
