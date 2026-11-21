from gpiozero import PWMLED
import tkinter as tk

# Setting up the LED on GPIO22
led = PWMLED(22)


# This function changes the LED brightness using the slider value
def change_brightness(value):
    brightness = float(value) / 100
    led.value = brightness


# Creating the main GUI window
window = tk.Tk()
window.title("Living Room Light")
window.geometry("500x300")


# Adding the title at the top of the window
title = tk.Label(
    window,
    text="Living Room Light",
    font=("Arial", 22)
)
title.pack(pady=20)


# Creating a slider to control the brightness from 0% to 100%
slider = tk.Scale(
    window,
    from_=0,
    to=100,
    orient=tk.HORIZONTAL,
    length=350,
    label="Brightness (%)",
    command=change_brightness
)
slider.set(50)
slider.pack(pady=20)


# Turning the LED off before closing the program
def close_program():
    led.off()
    window.destroy()


# Running the close function when the window is closed
window.protocol("WM_DELETE_WINDOW", close_program)


# Keeping the GUI running
window.mainloop()