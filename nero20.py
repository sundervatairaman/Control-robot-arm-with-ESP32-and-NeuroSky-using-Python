import socket
import json
import time
import numpy as np
import matplotlib.pyplot as plt
import asyncio
import websockets



# Define the server address and port
SERVER_ADDRESS = "127.0.0.1"
SERVER_PORT = 13854

async def send_websocket_message(message):
    uri = "ws://192.168.5.213:81"
    try:
        async with websockets.connect(uri) as websocket:
            await websocket.send(message)
    except ConnectionRefusedError:
        print("Connection was refused. Make sure the WebSocket server is running and accessible.")
    except Exception as e:
        print(f"An error occurred: {e}")


# Build the command to enable JSON output
config_packet = '{"enableRawOutput": true, "format": "Json"}'

# Function to parse JSON data
def parse_json(data, ax_theta, ax_attention):
    try:
        json_data = json.loads(data)
        #print(json_data)
        if 'eegPower' in json_data:
            eegPower_data = json_data['eegPower']


            delta = eegPower_data.get('delta')
            theta = eegPower_data.get('theta')
            lowAlpha = eegPower_data.get('lowAlpha')
            highAlpha = eegPower_data.get('highAlpha')
            lowBeta = eegPower_data.get('lowBeta')
            highBeta = eegPower_data.get('highBeta')

            lowGamma = eegPower_data.get('lowGamma')

            highGamma = eegPower_data.get('highGamma')


            print("highGamma:", highGamma)
            ax_theta.scatter(theta, highGamma)

            #ax.scatter(len(highGamma_values), highGamma)
            #plt.pause(0.01)
            highGamma_values.append(highGamma)
            theta_values.append(theta)
        if 'eSense' in json_data:
            eSense_data = json_data['eSense']

            attention = eSense_data.get('attention')
            meditation = eSense_data.get('meditation')

            attention_values.append(attention)
            meditation_values.append(meditation)
            print("attention:", attention,'meditation',meditation)

            ax_attention.scatter(attention, meditation)
            plt.pause(0.01)
            y=meditation/5
            x=(attention/10)+8
            z=1
            data = {
            "x": int(x),
            "y": int(y),
            "Z": int(z)  # Note: Use 'Z' instead of 'z' to match the JavaScript code
            }

            message = json.dumps(data)
            if x > 0: 
               asyncio.get_event_loop().run_until_complete(send_websocket_message(message))



    except json.JSONDecodeError as e:
        print("Error decoding JSON data:", e)

# Create a TCP socket and connect to the server
try:
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((SERVER_ADDRESS, SERVER_PORT))
    stream = client_socket.makefile(mode='rw')
    
    # Sending configuration packet to TGC
    stream.write(config_packet + '\r\n')
    stream.flush()

    time.sleep(5)  # Wait for a few seconds
    
    # Initialize plotting
    plt.ion()
    fig, (ax1, ax2) = plt.subplots(2, 1)
    ax1.set_xlabel('theta')
    ax1.set_ylabel('High Gamma')
    ax2.set_xlabel('attention')
    ax2.set_ylabel('meditation')


    highGamma_values = []
    theta_values = []
    attention_values = []
    meditation_values = []


    # Read packets and parse JSON data
    print("Reading bytes")
    while True:
        data = stream.readline().strip()
        parse_json(data, ax1, ax2)

except socket.error as se:
    print("Socket error:", se)

finally:
    if 'client_socket' in locals():
        client_socket.close()
