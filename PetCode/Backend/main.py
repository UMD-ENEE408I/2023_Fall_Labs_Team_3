import numpy as np
import cv2 as cv
from matplotlib import pyplot as plt
from matplotlib import patches
import socket
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
seconds = 0.5
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



#main loop
while(True):
    ret, frame = cam.read()
    cv.imshow('frame', frame)
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

#cleanup
cam.release()
cv.destroyAllWindows()
stream.stop_stream()
stream.close()
p.terminate()