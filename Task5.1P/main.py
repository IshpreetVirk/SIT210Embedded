
import tkinter as tk
from gpiozero import LED

# Set the GPIO pins connected to the three room lights
living_room = LED(17)
bathroom = LED(27)
closet = LED(22)

# This function switches on the selected room light
def change_light():
    selected = room.get()

    # Switch off all lights before selecting another room
    living_room.off()
    bathroom.off()
    closet.off()

    # Switch on the light for the selected room
    if selected == "Living Room":
        living_room.on()

    elif selected == "Bathroom":
        bathroom.on()

    elif selected == "Closet":
        closet.on()


# This function closes the application safely
def close_window():
    living_room.off()
    bathroom.off()
    closet.off()

    living_room.close()
    bathroom.close()
    closet.close()

    root.destroy()


# Create the main GUI window
root = tk.Tk()
root.title("Smart Home Lights")
root.geometry("400x300")
root.resizable(False, False)

# Display the heading
heading = tk.Label(
    root,
    text="Smart Home Lights",
    font=("Arial", 14)
)
heading.pack(pady=15)

# Store the selected room
room = tk.StringVar(value="")

# Create the three radio buttons
living_button = tk.Radiobutton(
    root,
    text="Living Room",
    variable=room,
    value="Living Room",
    command=change_light
)
living_button.pack()

bathroom_button = tk.Radiobutton(
    root,
    text="Bathroom",
    variable=room,
    value="Bathroom",
    command=change_light
)
bathroom_button.pack()

closet_button = tk.Radiobutton(
    root,
    text="Closet",
    variable=room,
    value="Closet",
    command=change_light
)
closet_button.pack()

# Create the Exit button
exit_button = tk.Button(
    root,
    text="Exit",
    command=close_window
)
exit_button.pack(pady=20)

# Safely close the application when the window is closed
root.protocol("WM_DELETE_WINDOW", close_window)

# Start the graphical interface
root.mainloop()
