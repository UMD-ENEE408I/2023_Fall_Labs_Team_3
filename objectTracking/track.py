import numpy as np
import cv2 as cv
import socket

###SETUP
#wifi
#REMEMBER TO CHANGE IP AND PORT!!
UDP_IP = "192.168.13.3"
UDP_PORT = 2390
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 

#camera
cam_index = 0
cam = cv.VideoCapture(cam_index)

#Yolo
net = cv.dnn.readNetFromDarknet('objectTracking\\yolov3.cfg', 'objectTracking\\yolov3.weights')
net.setPreferableBackend(cv.dnn.DNN_BACKEND_OPENCV)
classes = open('objectTracking\\coco.names').read().strip().split('\n')
ln = net.getLayerNames()
net.getUnconnectedOutLayers()
ln = [ln[i - 1] for i in net.getUnconnectedOutLayers()]

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
    h, w = img.shape[:2]

    for output in outputs:
        for detection in output:
            scores = detection[5:]
            classID = np.argmax(scores)
            confidence = scores[classID]
            if classes[classID] == 'person':
                if confidence > 0.5:
                    box = detection[:4] * np.array([w, h, w, h])
                    (centerX, centerY, width, height) = box.astype("int")
                    x = int(centerX - (width / 2))
                    y = int(centerY - (height / 2))
                    box = [x, y, int(width), int(height)]
                    boxes.append(box)
                    confidences.append(float(confidence))
    
    indices = cv.dnn.NMSBoxes(boxes, confidences, 0.5, 0.4)

    #get detected areas
    if len(indices) > 0:
        print("detected")
        for i in indices.flatten():
            (x, y) = (boxes[i][0], boxes[i][1])
            (w, h) = (boxes[i][2], boxes[i][3])
            cv.rectangle(img, (x, y), (x + w, y + h), (0,0,255), 2)
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
