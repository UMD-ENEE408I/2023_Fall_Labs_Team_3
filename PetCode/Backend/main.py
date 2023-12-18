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

###############################################  setup  ##################################
print('setup')
state = 'wait'
#camera
cam = cv.VideoCapture(0)
print('cam')

#signal
def signal_handler(sig, frame):
    print(state)
    # cleanup()
    # sys.exit()

signal.signal(signal.SIGINT, signal_handler)

# #audio
def callback(recognizer_instance, Audiodata):
    try:
        global state 
        state = 'wait'
        text = recognizer_instance.recognize_google(Audiodata)
        print(text)
        if 'speak' in text:
            state = 'speak'
            print('speak')
        if 'cow' in text:
            state = 'dog'
            print('finding cow')
        if 'giraffe' in text:
            state = 'giraffe'
            print('finding giraffe')
        if 'stop' in text:
            state = 'wait'
        if 'wait' in text:
            state = 'wait'
        if 'turn' in  text:
            state = 'turn'
    except sr.UnknownValueError:  
        print("could not understand audio")  
    except sr.RequestError as e:  
        print("error; {0}".format(e))

r = sr.Recognizer()
m = sr.Microphone()
with m as source:  
  r.adjust_for_ambient_noise(source, 1)

stop = r.listen_in_background(m, callback, phrase_time_limit = 2)

# chunk = 1024
# sample_format = pyaudio.paInt16
# channels = 1
# fs = 44100
# p1 = pyaudio.PyAudio()
# p2 = pyaudio.PyAudio()
# stream1 = p1.open(format=pyaudio.paInt16,
#                 channels=1,
#                 rate=44100,
#                 input=True,
#                 frames_per_buffer=chunk,
#                 input_device_index=4)
# stream2 = p2.open(format=pyaudio.paInt16,
#                 channels=1,
#                 rate=fs,
#                 input=True,
#                 frames_per_buffer=chunk,
#                 input_device_index=1)

#wifi
UDP_IP = "192.168.145.251"
UDP_PORT = 2390

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 

print('load yolo')
#Yolo
net = cv.dnn.readNetFromDarknet('yolov3.cfg', 'yolov3.weights')
net.setPreferableBackend(cv.dnn.DNN_BACKEND_OPENCV)
classes = open('coco.names').read().strip().split('\n')
ln = net.getLayerNames()
net.getUnconnectedOutLayers()
ln = [ln[i - 1] for i in net.getUnconnectedOutLayers()]
print('yolo')

#################################################  functions  ####################################
print('functions')
# def record(seconds):
#     frames1 = []
#     frames2 = []
#     for i in range(0, int(fs/chunk *seconds)):
#         data1 = stream1.read(chunk)
#         data2 = stream2.read(chunk)
#         for x in np.frombuffer(data1, dtype=np.int16):
#             frames1.append(x)
#         for x in np.frombuffer(data2, dtype=np.int16):
#             frames2.append(x)
#         frame1 = np.array(frames1)
#         frame2 = np.array(frames2)
#     return frame1, frame2

#send command
def sendCommand(command):
    MESSAGE = bytes(command, 'utf-8')
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    # data = sock.recv(128)
    # print(data)

# aux functions  ###########################

#turn some degrees
def turn(angle):
    MESSAGE = 'b'
    if(angle > 5):
        angle = angle
        MESSAGE = f'l{angle}'
    if(angle < -5):
        angle = -1*angle
        MESSAGE = f'r{angle}'
    sendCommand(MESSAGE)

#find angle of object
def find(target):
    _, img = cam.read()
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
            if target == 'dog':
                if classes[classID] == target or classes[classID] == 'cow':
                    if confidence > 0.2:
                        box = detection[:4] * np.array([imgw, imgh, imgw, imgh])
                        (centerX, centerY, width, height) = box.astype("int")
                        x = int(centerX - (width / 2))
                        y = int(centerY - (height / 2))
                        box = [x, y, int(width), int(height)]
                        boxes.append(box)
                        confidences.append(float(confidence))
            else:
                if classes[classID] == target:
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
    #get detected areas
    if len(indices) > 0:
        for i in indices.flatten():
            (x, y) = (boxes[i][0], boxes[i][1])
            (w, h) = (boxes[i][2], boxes[i][3])
            cv.rectangle(img, (x, y), (x + w, y + h), (0,0,255), 2)
        #calculate x position of detected object
        xpos = (x+w/2)
        return xpos
    return -1

def forward(distance):
    sendCommand(f'f{distance}')

# state functions  #####################
def speak():
    sendCommand('s')
    return 'wait'

found = False
counter = 0
wait_time = 0.75
def goto(target, found, counter):
    angle = find(target)
    if angle < 0:
        turn(90)
        time.sleep(wait_time)
        return target
    else:
        #calculate angle
        angle =  60-(120*angle/640)
        print(angle)
        turn(angle)
        time.sleep(wait_time)
        forward(5)
        return 'wait'
    #find initial angle
    # angle = find(target)
    # if(angle < 0):
    #     if found == True:
    #         found = False
    #         counter = 0
    #         return 'wait'
    #     else:
    #         if counter < 3:
    #             counter += 1
    #             turn(90)
    #             time.sleep(wait_time)
    #             return target
    #         return 'wait'
            # else: 
            #     counter = 0
            #     forward(5)
            #     time.sleep(wait_time)
            #     return target
    # else:
    #     found = True
    #     #calculate angle
    #     angle =  60-(120*angle/640)
    #     print(angle)
    #     turn(angle)
    #     time.sleep(wait_time)
    #     forward(2)
    #     return target

# Cleanup function  ###############
def cleanup():
    stop(wait_for_stop=False)
    time.sleep(3)
    cam.release()
    cv.destroyAllWindows()
    # stream1.stop_stream()
    # stream1.close()
    # p1.terminate()
    # stream2.stop_stream()
    # stream2.close()
    # p2.terminate()
    print('quiting')

##########################################################  main loop  ######################################
print('loop')
while(True):
    time.sleep(0.05)
    #ret, img = cam.read()
    match state:
        case 'wait':
            #do nothing
            state = 'wait'
        case 'speak':
            print('speaking')
            state = speak()
        case 'dog':
            state = goto('dog', found, counter)
        case 'giraffe':
            state = goto('giraffe', found, counter)
        case 'turn':
            turn(90)
            state = 'wait'

    # cv.imshow('frame', img)
    # if cv.waitKey(1) & 0xFF == ord('q'):
    #     break

############################################  cleanup  #########################
cleanup()