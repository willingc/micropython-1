"""
    led_dance.py
    ~~~~~~~~~~~~

    Light LEDs at random and fade them over time

    Usage: led_dance(delay) where
        'delay is the time between each new LED being turned on

    TODO: The random number generator is not great. Perhaps the accelerometer
          or compass could be used to add entropy.
"""
import microbit


def led_dance(delay):
    """ Light and fade LEDs at random with a time delay """
    dots = [[0] * 5, [0] * 5, [0] * 5, [0] * 5, [0] * 5]
    while True:
        dots[microbit.random(5)][microbit.random(5)] = 8
        for i in range(5):
            for j in range(5):
                microbit.display.set_pixel(i, j, dots[i][j])
                dots[i][j] = max(dots[i][j] - 1, 0)
        microbit.sleep(delay)

# main
led_dance(100)
