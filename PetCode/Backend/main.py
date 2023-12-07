import numpy as np
import cv2 as cv
from matplotlib import pyplot as plt
from matplotlib import patches
import socket
from scipy.fft import fft, fftshift, fftfreq
from scipy import signal
import pyaudio
import wave

from pocketsphinx import LiveSpeech
import speech_recognition as sr 

#######setup

#camera
cam = cv.VideoCapture(2)

#audio
chunk = 1024
sample_format = pyaudio.paInt16
channels = 1
fs = 44100
p1 = pyaudio.PyAudio()
p2 = pyaudio.PyAudio()
stream1 = p1.open(format=pyaudio.paInt16,
                channels=18,
                rate=44100,
                input=True,
                frames_per_buffer=chunk,
                input_device_index=1)
stream2 = p2.open(format=pyaudio.paInt16,
                channels=18,
                rate=fs,
                input=True,
                frames_per_buffer=chunk,
                input_device_index=2)

#wifi
UDP_IP = "192.168.145.251"
UDP_PORT = 2390
MESSAGE = b's'

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)
print("message: %s" % MESSAGE)

#functions
def record(seconds):
    frame1 = []
    frame2 = []
    for i in range(0, int(fs/chunk *seconds)):
        data1 = stream1.read(chunk)
        data2 = stream2.read(chunk)
        for x in np.frombuffer(data1, dtype=np.int16):
            frame1.append(x)
        for x in np.frombuffer(data2, dtype=np.int16):
            frame2.append(x)
        frame1 = np.array(frames)
        frame2 = np.array(frames)
    return frame1, frame2

def getDirection(m1, m2):
    #get time diff
    max = 0
    sign = 1
    index = 0
    for i in range(len(m1)):
        sum = np.sum(np.multiply(m1[0:i+1],m2[(len(m1)-1-i):len(m1)]))
        if max < sum:
            index = i
            max = sum
    for i in range(len(m1)):
        sum = np.sum(np.multiply(m2[0:i+1],m1[(len(m1)-1-i):len(m1)]))
        if max < sum:
            index = i
            max = sum
            sign = -1

    #delay = m2 shift
    delay = (len(m1)-index-1)*sign
    time_delay = delay/f
    angle = np.arcsin(time_delay*344/0.085)
    return angle

#set states
states = ['wait','listen','find','speak','follow']
state = states[0]

#main loop
while(True):
    ret, frame = cam.read()
    cv.imshow('frame', frame)
    match state:
        case 'wait':

        case 'listen':

        case 'find':

        case 'speak':

        case 'follow':


    if cv.waitKey(1) & 0xFF == ord('q'):
        break

#cleanup
cam.release()
cv.destroyAllWindows()
stream.stop_stream()
stream.close()
p.terminate()