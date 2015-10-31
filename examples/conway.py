"""
   conway.py
   ~~~~~~~~~

   Conway's Game of Life
   Press 'A' button or tap micro:bit to start a fresh layout
"""
import microbit

arena1 = bytearray(7 * 7)
arena2 = bytearray(7 * 7)

def show():
    img = microbit.Image(5,5)
    for y in range(5):
        for x in range(5):
            img.set_pixel(x, y, arena1[8 + y * 7 + x]*9)
    microbit.display.print(img)

def conway_step():
    """ Do one iteration of the game """
    global arena1, arena2
    for i in range(5 * 5): # loop over pixels
        i = 8 + (i // 5) * 7 + i % 5
        # count number of neigbours
        num_neighbours = (arena1[i - 8] +
                arena1[i - 7] +
                arena1[i - 6] +
                arena1[i - 1] +
                arena1[i + 1] +
                arena1[i + 6] +
                arena1[i + 7] +
                arena1[i + 8])
        # check if the centre cell is alive or not
        self = arena1[i]
        # apply the rules of life
        if self and not (2 <= num_neighbours <= 3):
            arena2[i] = 0 # not enough, or too many neighbours: cell dies
        elif not self and num_neighbours == 3:
            arena2[i] = 1 # exactly 3 neigbours around an empty cell: cell is born
        else:
            arena2[i] = self # stay as-is
    # swap the buffers (arena1 is now the new one to display)
    arena1, arena2 = arena2, arena1

while True:
    # randomise the start
    for i in range(5 * 5): # loop over pixels
        i = 8 + (i // 5) * 7 + i % 5
        arena1[i] = microbit.random(2) # set the pixel randomly
    show()
    microbit.sleep(1) # need to yield to update accelerometer (not ideal...)

    # loop while button a is not pressed
    while not microbit.button_a.is_pressed() and microbit.accelerometer.get_z() < -800:
        conway_step()
        show()
        microbit.sleep(150)
