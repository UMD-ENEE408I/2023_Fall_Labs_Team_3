import speech_recognition as sr
import signal
import sys

text = 'text'
prev = text

def signal_handler(sig, frame):
    print('exiting')
    print(text)
    stop(wait_for_stop=False)
    sys.exit()

signal.signal(signal.SIGINT, signal_handler)

def callback(recognizer_instance, Audiodata):
    try:
        global text 
        text = recognizer_instance.recognize_google(Audiodata)
        print(text)
    except sr.UnknownValueError:  
        print("could not understand audio")  
    except sr.RequestError as e:  
        print("error; {0}".format(e))  

recognizer = sr.Recognizer()
microphone = sr.Microphone()
with microphone as source:  
  recognizer.adjust_for_ambient_noise(source)

stop = recognizer.listen_in_background(microphone, callback)

print('say smth')

while(1):
    if(prev != text):
        print(text)
        prev = text

