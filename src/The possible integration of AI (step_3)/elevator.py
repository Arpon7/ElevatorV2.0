import tkinter as tk
import time
import threading

# Elevator State
current_floor = 0
direction = "IDLE"  # UP, DOWN, IDLE
requests = set()

# GUI Update Lock
lock = threading.Lock()

def add_request(floor):
    global requests
    with lock:
        requests.add(floor)
    update_display()

def scan_algorithm():
    global current_floor, direction, requests

    while True:
        time.sleep(1)

        with lock:
            if not requests:
                direction = "IDLE"
                update_display()
                continue

            up_queue = sorted([f for f in requests if f >= current_floor])
            down_queue = sorted([f for f in requests if f < current_floor], reverse=True)

        if direction in ["IDLE", "UP"]:
            direction = "UP"
            for floor in up_queue:
                move_to_floor(floor)

            direction = "DOWN"
            for floor in down_queue:
                move_to_floor(floor)

        else:
            direction = "DOWN"
            for floor in down_queue:
                move_to_floor(floor)

            direction = "UP"
            for floor in up_queue:
                move_to_floor(floor)


def move_to_floor(target):
    global current_floor, requests

    while current_floor != target:
        time.sleep(1)

        with lock:
            if current_floor < target:
                current_floor += 1
            elif current_floor > target:
                current_floor -= 1

        update_display()

    # Arrived
    with lock:
        if target in requests:
            requests.remove(target)

    open_doors()


def open_doors():
    status_var.set(f"Floor {current_floor} - Doors OPEN")
    time.sleep(2)
    status_var.set(f"Floor {current_floor} - Doors CLOSED")


def update_display():
    floor_var.set(f"Current Floor: {current_floor}")
    dir_var.set(f"Direction: {direction}")
    queue_var.set(f"Queue: {sorted(list(requests))}")


# GUI Setup
root = tk.Tk()
root.title("Elevator Simulation (SCAN Algorithm)")
root.geometry("400x400")

floor_var = tk.StringVar()
dir_var = tk.StringVar()
queue_var = tk.StringVar()
status_var = tk.StringVar()

tk.Label(root, textvariable=floor_var, font=("Arial", 14)).pack()
tk.Label(root, textvariable=dir_var, font=("Arial", 14)).pack()
tk.Label(root, textvariable=queue_var, font=("Arial", 12)).pack()
tk.Label(root, textvariable=status_var, font=("Arial", 12)).pack()

# Buttons for floors
for i in range(4):
    tk.Button(root, text=f"Call Floor {i}",
              command=lambda i=i: add_request(i),
              width=20).pack(pady=5)

update_display()

# Start SCAN in separate thread
threading.Thread(target=scan_algorithm, daemon=True).start()

root.mainloop()

