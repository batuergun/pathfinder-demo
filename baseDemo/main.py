import tkinter as tk
import socket
import pygame
from pygame.locals import *

TCP_IP = '192.168.1.100'
TCP_PORT = 8080

# Create a tkinter window
window = tk.Tk()
window.title("Motor Control")
window.geometry("1400x700")

# Create sliders for motor values
motor_sliders = []
for i in range(1, 9):
    slider = tk.Scale(window, from_=0, to=100,
                      orient=tk.HORIZONTAL, label=f"Motor {i}")
    slider.pack()
    motor_sliders.append(slider)

# Initialize Pygame and Joystick
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() > 0:
    joystick = pygame.joystick.Joystick(0)
    joystick.init()
else:
    print("No joystick connected")
    exit(1)

def send_motor_values():
    # Read the current slider values
    motor_values = [slider.get() for slider in motor_sliders]

    # Create a TCP socket connection
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((TCP_IP, TCP_PORT))

    # Format the motor values as a comma-separated string
    motor_string = ",".join(str(value) for value in motor_values)

    try:
        # Send the motor values over the socket
        sock.sendall(motor_string.encode('utf-8'))

        # Receive the response from the server
        response = sock.recv(1024)
        print(f"Received response: {response.decode('utf-8')}")

        # Update the debug output
        debug_output.delete(1.0, tk.END)
        debug_output.insert(tk.END, f"Sent motor values: {motor_string}")

    except Exception as e:
        print(f"Error occurred: {str(e)}")

    finally:
        # Close the socket connection
        sock.close()

# Create a button to send motor values
send_button = tk.Button(window, text='Send Motor Values',
                        command=send_motor_values)
send_button.pack()

# Create a text widget for debug output
debug_output = tk.Text(window, height=4, width=40)
debug_output.pack()

def update_debug_output(dummy_arg=None):
    # Read the current slider values
    motor_values = [slider.get() for slider in motor_sliders]

    # Update the debug output
    debug_output.delete(1.0, tk.END)
    debug_output.insert(tk.END, f"Motor values: {motor_values}")

# Update the debug output whenever the slider values change
for slider in motor_sliders:
    slider.config(command=update_debug_output)

def update_motor_values_from_joystick():
    # Read the joystick axes
    left_x_axis = joystick.get_axis(0)
    left_y_axis = joystick.get_axis(1)
    right_x_axis = joystick.get_axis(2)
    right_y_axis = joystick.get_axis(3)

    # Convert joystick axes values to motor values
    motor_values = [50 + int(50 * axis_value) for axis_value in [left_x_axis, left_y_axis, right_x_axis, right_y_axis]]

    # Update the sliders with the new motor values
    for slider, value in zip(motor_sliders, motor_values):
        slider.set(value)

    # Update the debug output
    update_debug_output()

while True:
    window.update_idletasks()
    window.update()

    # Process Pygame events
    for event in pygame.event.get():
        if event.type == QUIT:
            pygame.quit()
            sys.exit()

    # Update motor values from joystick input
    update_motor_values_from_joystick()
