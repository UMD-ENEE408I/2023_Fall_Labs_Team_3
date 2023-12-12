import numpy as np
import cv2 as cv
from matplotlib import pyplot as plt
from matplotlib import patches
import socket
from scipy.fft import fft, fftshift, fftfreq
from scipy import signal
import pyaudio
import wave
import signal
import sys
import time

import speech_recognition as sr 

#######setup
print('setup')
state = 'wait'
#camera
cam = cv.VideoCapture(1)
print('cam')

#signal
def signal_handler(sig, frame):
    print(state)
    # cleanup()
    # sys.exit()

signal.signal(signal.SIGINT, signal_handler)

#audio
def callback(recognizer_instance, Audiodata):
    try:
        global state 
        state = 'wait'
        text = recognizer_instance.recognize_google(Audiodata)
        print(text)
        if 'speak' in text:
            state = 'speak'
            print('speak')
        if 'find' in text:
            state = 'find'
            print('find')
        if 'stop' in text:
            state = 'wait'
        if 'wait' in text:
            state = 'wait'
    except sr.UnknownValueError:  
        print("could not understand audio")  
    except sr.RequestError as e:  
        print("error; {0}".format(e))

r = sr.Recognizer()
m = sr.Microphone()
with m as source:  
  r.adjust_for_ambient_noise(source, 2)

print('threadshit')
stop = r.listen_in_background(m, callback)

chunk = 1024
sample_format = pyaudio.paInt16
channels = 1
fs = 44100
p1 = pyaudio.PyAudio()
p2 = pyaudio.PyAudio()
stream1 = p1.open(format=pyaudio.paInt16,
                channels=1,
                rate=44100,
                input=True,
                frames_per_buffer=chunk,
                input_device_index=4)
stream2 = p2.open(format=pyaudio.paInt16,
                channels=1,
                rate=fs,
                input=True,
                frames_per_buffer=chunk,
                input_device_index=1)

#wifi
UDP_IP = "192.168.145.3"
UDP_PORT = 2390

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 

print('load yolo')
#Yolo
net = cv.dnn.readNetFromDarknet('objectTracking\\yolov3.cfg', 'objectTracking\\yolov3.weights')
net.setPreferableBackend(cv.dnn.DNN_BACKEND_OPENCV)
classes = open('objectTracking\\coco.names').read().strip().split('\n')
ln = net.getLayerNames()
net.getUnconnectedOutLayers()
ln = [ln[i - 1] for i in net.getUnconnectedOutLayers()]
print('yolo')
#functions
def record(seconds):
    frames1 = []
    frames2 = []
    for i in range(0, int(fs/chunk *seconds)):
        data1 = stream1.read(chunk)
        data2 = stream2.read(chunk)
        for x in np.frombuffer(data1, dtype=np.int16):
            frames1.append(x)
        for x in np.frombuffer(data2, dtype=np.int16):
            frames2.append(x)
        frame1 = np.array(frames1)
        frame2 = np.array(frames2)
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
    time_delay = delay/fs
    x = time_delay*344/0.085
    if(x > 1):
        x = 1
    if(x< -1):
        x=-1
    angle = np.arcsin(x)
    return angle*57.3

def turn(angle):
    MESSAGE = 'b'
    if(angle > 10):
        angle = angle
        MESSAGE = f'l{angle}'
    if(angle < -10):
        angle = -1*angle
        MESSAGE = f'r{angle}'
    print(MESSAGE)
    MESSAGE = bytes(MESSAGE, 'utf-8')
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

def speak():
    MESSAGE = b's'
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    print("sent")
    return 'wait'

def find():
    blob = cv.dnn.blobFromImage(img, 1/255.0, (416, 416), swapRB=True, crop=False)
    r = blob[0, 0, :, :]
    net.setInput(blob)
    outputs = net.forward(ln)

    boxes = []
    confidences = []
    imgh, imgw = img.shape[:2]
    for output in outputs:
        for detection in output:
            scores = detection[5:]
            classID = np.argmax(scores)
            confidence = scores[classID]
            if classes[classID] == 'dog':
                if confidence > 0.2:
                    box = detection[:4] * np.array([imgw, imgh, imgw, imgh])
                    (centerX, centerY, width, height) = box.astype("int")
                    x = int(centerX - (width / 2))
                    y = int(centerY - (height / 2))
                    box = [x, y, int(width), int(height)]
                    boxes.append(box)
                    confidences.append(float(confidence))
    
    indices = cv.dnn.NMSBoxes(boxes, confidences, 0.5, 0.4)

    x = 0
    w = 0
    MESSAGE = 'b'
    next_state = 'wait'
    #get detected areas
    if len(indices) > 0:
        for i in indices.flatten():
            (x, y) = (boxes[i][0], boxes[i][1])
            (w, h) = (boxes[i][2], boxes[i][3])
            cv.rectangle(img, (x, y), (x + w, y + h), (0,0,255), 2)
        #calculate x position of detected object
        xpos = (x+w/2)
        #calculate angle
        angle = -60
        angle = angle + 120*xpos/640
        if(angle > 10):
            angle = round(angle/10)
            MESSAGE = f'l{angle}'
            next_state = 'find'
        if(angle < -10):
            angle = -1*round(angle/10)
            MESSAGE = f'r{angle}'
            next_state = 'find'

    #send control to bot
    MESSAGE = bytes(MESSAGE, 'utf-8')
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    print("sent" + str(MESSAGE))
    return next_state
print('func')
def cleanup():
    stop(wait_for_stop=False)
    time.sleep(3)
    cam.release()
    cv.destroyAllWindows()
    stream1.stop_stream()
    stream1.close()
    p1.terminate()
    stream2.stop_stream()
    stream2.close()
    p2.terminate()
    print('quiting')
print('loop')
#main loop
while(True):
    time.sleep(0.05)
    ret, img = cam.read()
    cv.imshow('frame', img)
    match state:
        case 'wait':
            #do nothing
            state = 'wait'
        case 'speak':
            state = speak()
        case 'find':
            state = find()

    if cv.waitKey(1) & 0xFF == ord('q'):
        break

#cleanup
cleanup()