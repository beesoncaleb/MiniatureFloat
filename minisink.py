import requests
import matplotlib.pyplot as plt
import pandas as pd
from time import sleep
from datetime import datetime
import pytz

csv_counter = 0

miniserver = "http://192.168.4.1/"

def plotcsv(filename):
    # Read CSV
    dive_csv = pd.read_csv(filename)

    # Create a 4-row, 1-column subplot layout
    fig, axs = plt.subplots(3, 1, figsize=(10, 8), sharex=True)

    #depth over time plot
    axs[0].plot(dive_csv["time"], dive_csv["depth"], label="Depth", color="blue")
    axs[0].set_ylabel("Depth (m)")
    axs[0].set_title("Depth over Time")

    #pressure over time plot
    axs[1].plot(dive_csv["time"], dive_csv["pressure"], label="Pressure", color="green")
    axs[1].set_ylabel("Pressure (kPa)")
    axs[1].set_title("Pressure over Time")

    #temperature over time plot
    axs[2].plot(dive_csv["time"], dive_csv["temperature"], label="Temperature", color="red")
    axs[2].set_ylabel("Temp (°C)")
    axs[2].set_title("Temperature over Time")

    # Improve layout
    for ax in axs:
        ax.grid(True)
        ax.legend()

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    print("Establishing connection to float...")
    sleep(1)

    timeheader = {"Content-Type" : "text/plain"}
    central = pytz.timezone('CST6CDT')
    now = datetime.now(central)
    hour = now.hour
    minute = now.minute
    datastring = f"{hour:02d}:{minute:02d}"
    timecheck = requests.post(miniserver + "time", data=datastring, headers=timeheader)

    initial = requests.get(miniserver)
    print("Connection to float established")
    print("\nInitial Information:")
    print(initial.text)
    print("")

    while True:
        response = ""
        req = input("Enter a command: profile1 | profile2 | profile | plot | reset | exit ").upper()

        if req == "PROFILE1":
            print("Initiating depth profile 1")
            requests.get(miniserver + "profile1") 
        elif req == "PROFILE2":
            print("Initiating depth profile 2")
            requests.get(miniserver + "profile2") 
        elif req == "PROFILE":
            depth = None
            while True:
                try:
                    depth = float(input("Enter depth in meters: "))
                except ValueError:
                    continue
                break
            depth = str(depth)
            print(f"Initiating depth profile")
            requests.post(miniserver + "profile", data=depth, headers=timeheader)
        elif req == "PLOT":
            print("Retrieving data...")
            response = requests.get(miniserver + "plot")
            with open(f"dive_csv{csv_counter}.csv", "w") as csv:
                csv.write(response.text)
            plotcsv(f"dive_csv{csv_counter}.csv")
            csv_counter += 1
        elif req == "RESET":
            response = requests.get(miniserver + "reset")
            print(response.text, "\n")
        elif req == "EXIT":
            break
        else:
            print(f"\n{req} is not a valid command\n")