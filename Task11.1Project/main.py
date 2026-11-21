
import os
import json
import ssl
import smtplib
import tkinter as tk

from tkinter import ttk, messagebox
from dotenv import load_dotenv
from gpiozero import OutputDevice
from email.message import EmailMessage
from queue import Queue, Empty
import paho.mqtt.client as mqtt


# Load the MQTT, Gmail and GPIO settings from the .env file.
# This keeps passwords and connection details separate from the main program.
load_dotenv()

MQTT_BROKER = os.getenv("MQTT_BROKER")
MQTT_PORT = int(os.getenv("MQTT_PORT", 8883))
MQTT_USERNAME = os.getenv("MQTT_USERNAME")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD")

GMAIL_SENDER = os.getenv("GMAIL_SENDER")
GMAIL_APP_PASSWORD = os.getenv("GMAIL_APP_PASSWORD")
EMAIL_RECIPIENT = os.getenv("EMAIL_RECIPIENT")

BUZZER_GPIO = int(os.getenv("BUZZER_GPIO", 18))
EMAIL_COOLDOWN = int(os.getenv("EMAIL_COOLDOWN", 60))

CA_FILE = "emqx-ca.pem"
SETTINGS_FILE = "settings.json"

TEMP_TOPIC = "smartflow/temperature"
HUMIDITY_TOPIC = "smartflow/humidity"
LIGHT_TOPIC = "smartflow/light"
VIBRATION_TOPIC = "smartflow/vibration_pulses"
LED_TOPIC = "smartflow/red_led"


# These are the starting threshold values used when the dashboard opens.
# The user can change them from the dashboard and save the new settings.
settings = {
    "light_threshold": 100,
    "vibration_threshold": 5
}

if os.path.exists(SETTINGS_FILE):
    try:
        with open(SETTINGS_FILE, "r") as file:
            saved_settings = json.load(file)
            settings.update(saved_settings)
    except (json.JSONDecodeError, OSError):
        print("Could not load saved settings. Using default values.")


# The Raspberry Pi uses GPIO18 to control the buzzer.
# A suitable transistor driver should be used if the buzzer needs more current.
buzzer = OutputDevice(BUZZER_GPIO, active_high=True, initial_value=False)

message_queue = Queue()

temperature_value = "--"
humidity_value = "--"
light_value = "--"
vibration_value = "--"

last_email_time = 0
vibration_alert_active = False
mqtt_connected = False


# Create the MQTT client and configure its secure connection to EMQX.
# The CA certificate verifies the broker when the Raspberry Pi connects.
client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    client_id="NetHouseRaspberryPi"
)

client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
client.tls_set(
    ca_certs=CA_FILE,
    tls_version=ssl.PROTOCOL_TLS_CLIENT
)


def on_connect(client, userdata, flags, reason_code, properties):
    global mqtt_connected

    if reason_code == 0:
        mqtt_connected = True
        print("Connected to EMQX MQTT broker.")

        client.subscribe(TEMP_TOPIC)
        client.subscribe(HUMIDITY_TOPIC)
        client.subscribe(LIGHT_TOPIC)
        client.subscribe(VIBRATION_TOPIC)

    else:
        mqtt_connected = False
        print("MQTT connection failed:", reason_code)


def on_disconnect(client, userdata, disconnect_flags, reason_code, properties):
    global mqtt_connected

    mqtt_connected = False
    print("MQTT disconnected.")


def on_message(client, userdata, message):
    # MQTT messages arrive in the background, so they are placed in a queue.
    # The Tkinter dashboard reads this queue separately to avoid GUI thread issues.
    try:
        value = message.payload.decode("utf-8")
        message_queue.put((message.topic, value))
    except Exception as error:
        print("Message error:", error)


client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_message = on_message


def send_email(subject, body):
    # Gmail uses an App Password to allow this program to send notifications.
    # The normal Gmail account password should not be used here.
    if not all([GMAIL_SENDER, GMAIL_APP_PASSWORD, EMAIL_RECIPIENT]):
        print("Email settings are missing from the .env file.")
        return

    email = EmailMessage()
    email["Subject"] = subject
    email["From"] = GMAIL_SENDER
    email["To"] = EMAIL_RECIPIENT
    email.set_content(body)

    try:
        with smtplib.SMTP_SSL("smtp.gmail.com", 465) as server:
            server.login(GMAIL_SENDER, GMAIL_APP_PASSWORD)
            server.send_message(email)

        print("Email notification sent.")

    except Exception as error:
        print("Email sending failed:", error)


def save_settings():
    # Save the threshold values so they are still available next time.
    # Invalid entries are rejected instead of being saved.
    try:
        light_limit = float(light_entry.get())
        vibration_limit = int(vibration_entry.get())

        if light_limit < 0 or vibration_limit < 1:
            raise ValueError

        settings["light_threshold"] = light_limit
        settings["vibration_threshold"] = vibration_limit

        with open(SETTINGS_FILE, "w") as file:
            json.dump(settings, file, indent=4)

        messagebox.showinfo("Settings", "Thresholds saved successfully.")

    except ValueError:
        messagebox.showerror(
            "Invalid settings",
            "Enter a valid light value and a vibration threshold of at least 1."
        )


def turn_led_off():
    # The Raspberry Pi sends OFF after five seconds to switch off the Arduino LED.
    # The Arduino also has its own five-second safety timeout.
    if mqtt_connected:
        client.publish(LED_TOPIC, "OFF")


def stop_buzzer():
    # Stop the buzzer after the alert period has finished.
    buzzer.off()


def trigger_alert():
    # A vibration alert activates the buzzer and red LED for five seconds.
    # An email is also sent, subject to the configured notification cooldown.
    print("Vibration alert detected!")

    buzzer.on()
    root.after(5000, stop_buzzer)

    if mqtt_connected:
        client.publish(LED_TOPIC, "ON")
        root.after(5000, turn_led_off)

    global last_email_time
    current_time = __import__("time").time()

    if current_time - last_email_time >= EMAIL_COOLDOWN:
        last_email_time = current_time

        body = (
            "A vibration alert has been detected by the "
            "Net House Monitoring and Security System.\n\n"
            f"Vibration pulses: {vibration_value}\n"
            f"Temperature: {temperature_value} C\n"
            f"Humidity: {humidity_value} %\n"
            f"Light intensity: {light_value} lux\n"
        )

        send_email("Net House Security Alert", body)


def process_messages():
    # Read incoming sensor values and update the dashboard from the main GUI thread.
    # This also checks whether the vibration count has reached the selected threshold.
    global temperature_value, humidity_value
    global light_value, vibration_value, vibration_alert_active

    try:
        while True:
            topic, value = message_queue.get_nowait()

            if topic == TEMP_TOPIC:
                temperature_value = value
                temperature_label.config(text=f"{value} °C")

            elif topic == HUMIDITY_TOPIC:
                humidity_value = value
                humidity_label.config(text=f"{value} %")

            elif topic == LIGHT_TOPIC:
                light_value = value
                light_label.config(text=f"{value} lux")

                try:
                    if float(value) < float(settings["light_threshold"]):
                        light_status_label.config(
                            text="Low light detected",
                            foreground="orange"
                        )
                    else:
                        light_status_label.config(
                            text="Light level normal",
                            foreground="green"
                        )
                except ValueError:
                    light_status_label.config(text="Invalid light reading")

            elif topic == VIBRATION_TOPIC:
                vibration_value = value
                vibration_label.config(text=f"{value} pulses")

                try:
                    pulse_count = int(value)
                    vibration_limit = int(settings["vibration_threshold"])

                    if pulse_count >= vibration_limit:
                        if not vibration_alert_active:
                            vibration_alert_active = True
                            trigger_alert()
                    else:
                        vibration_alert_active = False

                except ValueError:
                    print("Invalid vibration reading:", value)

    except Empty:
        pass

    if mqtt_connected:
        connection_label.config(
            text="MQTT Connected",
            foreground="green"
        )
    else:
        connection_label.config(
            text="MQTT Disconnected",
            foreground="red"
        )

    root.after(100, process_messages)


def close_program():
    # Turn off the buzzer and LED before closing the dashboard.
    # The MQTT connection is also stopped cleanly.
    buzzer.off()

    if mqtt_connected:
        client.publish(LED_TOPIC, "OFF")

    client.loop_stop()
    client.disconnect()
    buzzer.close()
    root.destroy()


# Create the main dashboard window and display the latest sensor readings.
# The dashboard also provides fields for changing and saving alert thresholds.
root = tk.Tk()
root.title("Net House Monitoring and Security System")
root.geometry("600x650")
root.resizable(False, False)

title_label = ttk.Label(
    root,
    text="NET HOUSE MONITORING SYSTEM",
    font=("Arial", 18, "bold")
)
title_label.pack(pady=20)

connection_label = ttk.Label(
    root,
    text="MQTT Disconnected",
    font=("Arial", 11, "bold"),
    foreground="red"
)
connection_label.pack(pady=5)

sensor_frame = ttk.LabelFrame(root, text="Live Sensor Readings", padding=15)
sensor_frame.pack(fill="x", padx=30, pady=15)

ttk.Label(sensor_frame, text="Temperature:").grid(
    row=0, column=0, sticky="w", pady=8
)
temperature_label = ttk.Label(sensor_frame, text="-- °C")
temperature_label.grid(row=0, column=1, sticky="w", padx=20)

ttk.Label(sensor_frame, text="Humidity:").grid(
    row=1, column=0, sticky="w", pady=8
)
humidity_label = ttk.Label(sensor_frame, text="-- %")
humidity_label.grid(row=1, column=1, sticky="w", padx=20)

ttk.Label(sensor_frame, text="Light intensity:").grid(
    row=2, column=0, sticky="w", pady=8
)
light_label = ttk.Label(sensor_frame, text="-- lux")
light_label.grid(row=2, column=1, sticky="w", padx=20)

light_status_label = ttk.Label(
    sensor_frame,
    text="Waiting for reading",
    foreground="gray"
)
light_status_label.grid(row=2, column=2, padx=10)

ttk.Label(sensor_frame, text="Vibration:").grid(
    row=3, column=0, sticky="w", pady=8
)
vibration_label = ttk.Label(sensor_frame, text="-- pulses")
vibration_label.grid(row=3, column=1, sticky="w", padx=20)


settings_frame = ttk.LabelFrame(root, text="Alert Settings", padding=15)
settings_frame.pack(fill="x", padx=30, pady=15)

ttk.Label(settings_frame, text="Minimum light level (lux):").grid(
    row=0, column=0, sticky="w", pady=8
)

light_entry = ttk.Entry(settings_frame, width=15)
light_entry.grid(row=0, column=1, padx=10)
light_entry.insert(0, str(settings["light_threshold"]))

ttk.Label(settings_frame, text="Vibration threshold (pulses):").grid(
    row=1, column=0, sticky="w", pady=8
)

vibration_entry = ttk.Entry(settings_frame, width=15)
vibration_entry.grid(row=1, column=1, padx=10)
vibration_entry.insert(0, str(settings["vibration_threshold"]))

save_button = ttk.Button(
    settings_frame,
    text="Save Settings",
    command=save_settings
)
save_button.grid(row=2, column=0, columnspan=2, pady=15)


info_label = ttk.Label(
    root,
    text="Vibration alerts activate the buzzer and red LED for 5 seconds.",
    wraplength=500,
    justify="center"
)
info_label.pack(pady=15)

root.protocol("WM_DELETE_WINDOW", close_program)

# Start the MQTT connection in the background so the dashboard remains responsive.
# The GUI then checks for new sensor messages every 100 milliseconds.
try:
    client.connect(MQTT_BROKER, MQTT_PORT, keepalive=30)
    client.loop_start()
except Exception as error:
    print("Could not connect to MQTT:", error)

root.after(100, process_messages)
root.mainloop()
