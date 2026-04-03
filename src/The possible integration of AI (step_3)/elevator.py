import tkinter as tk
import time
import threading
import random
import csv
from datetime import datetime
import numpy as np
from sklearn.ensemble import IsolationForest


current_floor = 0
direction = "IDLE"
requests = set()

lock = threading.Lock()


X_train = np.array([
    [1.2, 2.0],
    [1.1, 2.1],
    [1.3, 1.9],
    [1.25, 2.0],
    [1.15, 2.05]
])

model = IsolationForest(contamination=0.1, random_state=42)
model.fit(X_train)

def detect_anomaly(current, door_time):
    pred = model.predict([[current, door_time]])
    return "FAULT" if pred[0] == -1 else "NORMAL"


LOG_FILE = "elevator_log.csv"

def init_logger():
    try:
        with open(LOG_FILE, "x", newline="") as f:
            writer = csv.writer(f)
            writer.writerow([
                "timestamp", "floor", "direction",
                "motor_current", "door_time", "status"
            ])
    except FileExistsError:
        pass

def log_data(floor, direction, motor_current, door_time, status):
    with open(LOG_FILE, "a", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            floor,
            direction,
            motor_current,
            door_time,
            status
        ])


def add_request(floor):
    global requests
    with lock:
        requests.add(floor)
    update_display()

def simulate_motor_current():
    current = round(random.uniform(1.0, 1.5), 2)

    # occasional fault
    if random.random() < 0.1:
        current += random.uniform(0.8, 1.5)

    return round(current, 2)

def simulate_door_time():
    t = round(random.uniform(1.8, 2.2), 2)


    if random.random() < 0.1:
        t += random.uniform(1.0, 2.0)

    return round(t, 2)

def move_to_floor(target):
    global current_floor, requests

    while current_floor != target:
        time.sleep(1)

        motor_current = simulate_motor_current()

        with lock:
            if current_floor < target:
                current_floor += 1
            elif current_floor > target:
                current_floor -= 1

        log_data(current_floor, direction, motor_current, None, "MOVING")

        update_display()


    with lock:
        if target in requests:
            requests.remove(target)

    open_doors()

def open_doors():
    door_time = simulate_door_time()
    motor_current = simulate_motor_current()

    status = detect_anomaly(motor_current, door_time)

    status_var.set(f"Floor {current_floor} - Doors OPEN ({status})")

    log_data(current_floor, direction, motor_current, door_time, status)

    time.sleep(door_time)

    status_var.set(f"Floor {current_floor} - Doors CLOSED")

def scan_algorithm():
    global direction

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


root = tk.Tk()
root.title("Elevator Simulation with AI + Data Logger")
root.geometry("420x450")

floor_var = tk.StringVar()
dir_var = tk.StringVar()
queue_var = tk.StringVar()
status_var = tk.StringVar()

tk.Label(root, textvariable=floor_var, font=("Arial", 14)).pack()
tk.Label(root, textvariable=dir_var, font=("Arial", 14)).pack()
tk.Label(root, textvariable=queue_var, font=("Arial", 12)).pack()
tk.Label(root, textvariable=status_var, font=("Arial", 12)).pack(pady=10)


for i in range(4):
    tk.Button(
        root,
        text=f"Call Floor {i}",
        command=lambda i=i: add_request(i),
        width=20
    ).pack(pady=5)

def update_display():
    floor_var.set(f"Current Floor: {current_floor}")
    dir_var.set(f"Direction: {direction}")
    queue_var.set(f"Queue: {sorted(list(requests))}")


init_logger()
update_display()

threading.Thread(target=scan_algorithm, daemon=True).start()

root.mainloop()