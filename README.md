(Morse)

.-- . .-.. -.-. --- -- . -.-.-- / - .... .. ... / .-- --- .-. -.- / .. ... / -.. --- -. . / ..-. --- .-. / ..-. ... -- -... / .-. . -.-. .-. ..- .. - -- . -. - / - . ... - .-.-.- / .... --- .-- . ...- . .-. --..-- / .- / ..-. ..- -. / .-- . . -.- . -. -.. / .--. .-. --- .--- . -.-. - / .. -. -.. . . -.. .-.-.- .-.-.- .-.-.- .-.-.-

-------------------------------------------------------------------------------------------------------------
ElevatorV2.0
This is a small project to explain how a lift (elevator) works. A four-story building is chosen as a test case. The repository mainly has 3 parts:

An interactive webpage to comprehend the algorithm that is used to run the elevator system efficiently between floors.
A wowki simulation
A Python-based demo for the datalogger (to enable predictive maintenance).
Please go through the full explanation to understand how it works!

-------------------------------------------------------------------------------------------------------------

Step 1: Understanding the algorithm:

I developed a web-based interface to simulate the elevator algorithm (SCAN algorithm). Lots of effort went into it (used html and JS). Please first visit the live site for an interactive simulation!

https://arpon7.github.io/InteractiveElevator-web/

For a detailed explanation of the algorithm, please follow the link: link to the site repo: https://github.com/Arpon7/InteractiveElevator-web

-------------------------------------------------------------------------------------------------------------

Step 2: Running the algorithm in a hardware:

In the real world, anelevator system is generally PLC based. This is a distributed system with a main controller. The key components are: Control System - decides which floor to go to and in what order Motor System - moves the elevator up and down Safety System - ensures doors close, weight limits aren't exceeded Call System - buttons inside and outside the elevator

Lots of hardware (with proper coordination) is required to build such a system. There is a motor drive to control the induction motor, a controller for user interactions, a safety and recovery system, a lighting and ventilation system, etc.

But we can focus on the control system for the smooth running between floors. A bare minimum setup is done using a great simulation platform, Wokwi. We can just hit the play button, and interact with the hardware!

This elevator serves 4 floors (Ground, 1, 2, 3) using the SCAN algorithm, which moves in one direction serving all requests before reversing. for the floor sensing sensor, push buttons are used. In the real world, limit switches do the job.
Wowki Simulation:

https://wokwi.com/projects/460287833026581505

Limitations of this simulation:

1. While the processor is busy executing one task, any other command gets disregarded. For example, if the lift is going from floor 0 to floor 2, any call from floor 3 will not register. Using a dedicated controller for the call buttons and then sending the requests on a priority basis to the main controller will solve the problem.
2. No safety features (such as IR sensors to prevent door shutdown if a movement is detected on the cab door rail during closing) are implemented.


-------------------------------------------------------------------------------------------------------------
Step 3: The possible integration of AI: In an elevator system, maintenance within a scheduled timeframe is mandatory. However, such routine maintainence effort can always be more effective if a central data logger is used to track the performance of all supporting components. Data logging enables: Trend analysis, Model training and Fault traceability. We can track:

Timestamp: Time of event

Current floor: Position

Direction: UP/DOWN/IDLE

Motor current: Simulated load

Door operation time: Health indicator

Travel time: Between floors

Fault flags: Detected anomalies

The provided elevator.py file simulates these parameters with the help of a GUI and creates a .CSV file as a datalogger. In the real world, an SBC (single-board computer) or a custom board with enough memory can handle this. Even such a setup can communicate with a remote server (cloud).



Work In Progress (WIP): As a stepping stone, this project is good. But the ultimate target is to build a product with features like Predictive Maintenance (PdM).

*Great Youtube video for the overall working of the elevator system: https://www.youtube.com/watch?v=rKp4pe92ljg
