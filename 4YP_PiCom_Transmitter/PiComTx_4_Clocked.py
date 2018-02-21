# Using GPIO library for Raspberry Pi
import RPi.GPIO as GPIO
from time import sleep
import threading
import paramiko

# Transmission frequency in Hz
transmit_freq = 100
clock_pin = 18
data_pin = 4

def Ssh_Start_Receiver():
    print("\n---SSH lock---")
    host = "raspberrypi2.local"
    uname = "pi"
    pword = "rasPass2"
    command = "sudo python3 /home/pi/Documents/PiComRx_3_Clocked.py"
    try:
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        # Connect to host
        print("Connecting to {}".format(host))
        ssh.connect(host, username=uname, password=pword)
        print("Connected!")
    except Exception as e:
        print("Exception raised attempting to connect to ssh host",
              "\nError is: {}".format(e))

    if ssh.get_transport().is_active():
        print("Executing command to start comReceiver")
        stdin, stdout, stderr = ssh.exec_command(command)
        
        sleep(5)
        print("Closing ssh connection\n")
        ssh.close()
        return True
    else:
        print("Closing unsuccessful ssh connection\n")
        ssh.close()
        return False

def Prep_Binary_Data(data_list):
    if not all(b == 0 or b == 1 for b in data_list):
        print("Data to transmit is not binary, stopping transmission")
    else:
        # Add excess continuous value padding
        print("SIZE OF INPUT = {}".format(len(data_list)))
        pad_bits=0
        for i in range(5,len(data_list)):
            if data_list[i+pad_bits-5:i+pad_bits] == [0]*5:
                data_list.insert(i+pad_bits,1)
                pad_bits = pad_bits + 1
            elif data_list[i+pad_bits-5:i+pad_bits] == [1]*5:
                data_list.insert(i+pad_bits,0)
                pad_bits = pad_bits + 1
        # Adding end start and end of transmission padding to data
        data_list[:0] = [1]*7+[0]
        data_list[-1:] = [0]+[1]*7
        print("SIZE OF PADDED INPUT = {}".format(len(data_list)))
        return data_list

def Transmit_Binary_Data(data_list):
        print("Transmitting data")
        ############################# STILL NEED TIMING#############
        i = 0
        for b in data_list:
            i = i+1
            GPIO.output(clock_pin,i%2)
            GPIO.output(data_pin, b)
            sleep(1/transmit_freq)
        GPIO.output(data_pin, GPIO.LOW)
        print("Data transmission complete!")

try:
    # Use BCM numbering standard
    GPIO.setmode(GPIO.BCM);
    # Set BCM pin 4 as an output
    GPIO.setup(data_pin, GPIO.OUT, initial=GPIO.LOW)
    GPIO.setup(clock_pin, GPIO.OUT, pull_up_down=GPIO.PUD_DOWN)

    receiver_started = Ssh_Start_Receiver()
    if receiver_started:
        input_stream = ([1,0]*5 + [1]*10)*20
        Prep_Binary_Data(input_stream)
        sleep(2)
        Transmit_Binary_Data(input_stream)

    print("Finishing program")
        
except KeyboardInterrupt:
    print("\nExiting program on keyboard interrupt")
except Exception as e:
    print("\nExiting program on unexpected error\nError is: {}".format(e))
finally:
    GPIO.cleanup()