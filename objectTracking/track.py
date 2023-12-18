import numpy as np
import cv2 as cv
import socket

###SETUP
#wifi
#REMEMBER TO CHANGE IP AND PORT!!
UDP_IP = "192.168.145.3"
UDP_PORT = 2390
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 

#camera
cam_index = 1
cam = cv.VideoCapture(cam_index)

#Yolo
net = cv.dnn.readNetFromDarknet('objectTracking\\yolov3.cfg', 'objectTracking\\yolov3.weights')
net.setPreferableBackend(cv.dnn.DNN_BACKEND_OPENCV)
classes = open('objectTracking\\coco.names').read().strip().split('\n')
ln = net.getLayerNames()
net.getUnconnectedOutLayers()
ln = [ln[i - 1] for i in net.getUnconnectedOutLayers()]

#controls
xpos = 0

###LOOP
while(1):
    #read img
    ret, frame = cam.read()
    #convert image to grayscale
    img = cv.cvtColor(frame, cv.COLOR_BGR2RGB)

    #find object
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
        if(angle < -10):
            angle = -1*round(angle/10)
            MESSAGE = f'r{angle}'

    #TODO: controls for bot

    #send control to bot
    MESSAGE = b = bytes(MESSAGE, 'utf-8')
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    print("sent" + str(MESSAGE))
    #display output image
    cv.imshow('frame', img)
    #exit loop
    if cv.waitKey(5) & 0xFF == ord('q'):
        break

###CLEANUP
#wifi
sock.shutdown(socket.SHUT_RDWR)
sock.close()

#camera
cv.destroyAllWindows()
cam.release()
