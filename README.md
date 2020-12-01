# Searching for Circles in a Bitmap File

This program processes a 200 x 200 BITMAP file containing up to 20 circles with no circle that touch nor overlap each other, would generate a Graphics Description Language file containing commands that creates all the circles preserving each of their radius and color but placing all of the circles' center at the center of the BITMAP.

This implements a depth-first search in going over the bitmap file and determining the radius of each circle.  Also, it uses Shell Sort to arrange the circles in order.

The output file can be used as input to the Graphics Description Language Interpreter.
