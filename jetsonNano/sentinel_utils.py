# MIT License
# Copyright (c) 2019-2022 JetsonHacks

# A simple code snippet
# Using two  CSI cameras (such as the Raspberry Pi Version 2) connected to a
# NVIDIA Jetson Nano Developer Kit with two CSI ports (Jetson Nano, Jetson Xavier NX) via OpenCV
# Drivers for the camera and OpenCV are included in the base image in JetPack 4.3+

# This script will open a window and place the camera stream from each camera in a window
# arranged horizontally.
# The camera streams are each read in their own thread, as when done sequentially there
# is a noticeable lag

import cv2
import threading
from threading import Thread, Timer
import numpy as np
import torch
import cv2
import numpy as np
from models.experimental import attempt_load
from utils.general import non_max_suppression
import qwiic_vl53l1x as qwiic
from time import sleep
import smbus2
import bme280

import time
import Jetson.GPIO as GPIO

from NanoLambdaNSP32 import *


device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

PinRst		= 13  	# pin Reset (the number is based on GPIO.BOARD)
PinReady	= 15 	# pin Ready (the number is based on GPIO.BOARD)

UP_pin = 33
Down_pin = 35
Pulse_pin = 37
Work_pin = 40



class temp_sensor():
    def __init__(self, save_dir, Port = 0, address = 0x76) -> None:
        self.port = Port
        self.address = address
        self.bus = smbus2.SMBus(self.port)
        self.calibration_params = bme280.load_calibration_params(self.bus, self.address)
        self.data = None
        self.save_dir = save_dir

    def read(self):
        self.data = bme280.sample(self.bus, self.address, self.calibration_params)
        print(self.data.id)
        print(self.data.timestamp)
        print(self.data.temperature)
        print(self.data.pressure)
        print(self.data.humidity)

    def save(self, filename):
        f = open(self.save_dir + filename, 'w')
        f.write(str(self.data.id) + '\n')
        f.write(str(self.data.timestamp) + '\n')
        f.write(str(self.data.temperature) + '\n')
        f.write(str(self.data.pressure) + '\n')
        f.write(str(self.data.humidity) + '\n')
        f.close()

    



class RepeatTimer(Timer):
    def run(self):
        while not self.finished.wait(self.interval):
            self.function(*self.args, **self.kwargs)


class PicoPins():
    def __init__(self, lock, Pulse):
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup(UP_pin, GPIO.OUT)
        GPIO.setup(Down_pin, GPIO.OUT)
        GPIO.setup(Pulse_pin, GPIO.OUT)
        #GPIO.setup(Work_pin, GPIO.OUT)



        #GPIO.output(Work_pin, GPIO.HIGH)


        self.lock = lock
        self.up = False
        self.down = False
        self.pulsePin = False
        if Pulse:
            self.timedPulse = RepeatTimer(1, self.toggle)
            self.timedPulse.start()


    def toggle(self):

        self.lock.acquire()

        self.pulsePin = not self.pulsePin
        
        if self.pulsePin:
            GPIO.output(Pulse_pin, GPIO.HIGH)
        else:
            GPIO.output(Pulse_pin, GPIO.LOW)

        print("toggle")

        self.lock.release()

    def  ins(self):


        if not self.up:
            GPIO.output(UP_pin, GPIO.LOW)

        else:
            GPIO.output(UP_pin, GPIO.HIGH)

        if not self.down:
            GPIO.output(Down_pin, GPIO.LOW)

        else:
            GPIO.output(Down_pin, GPIO.HIGH)

        print('update')



    def pins_up(self):
        GPIO.output(Down_pin, GPIO.LOW)
        #sleep(5)
        GPIO.output(UP_pin, GPIO.HIGH)
        self.up = True
        self.down = False
        #self.updatePins()


    def pins_down(self):
        GPIO.output(UP_pin, GPIO.LOW)
        #sleep(5)
        GPIO.output(Down_pin, GPIO.HIGH)
        self.down = True
        self.up = False
        #self.updatePins()

    def pins_stop(self):
        GPIO.output(UP_pin, GPIO.LOW)
        GPIO.output(Down_pin, GPIO.LOW)
        sleep(5)
        self.up = False
        self.down = False
        #self.updatePins()


class spectrumSensor:
    def __init__(self, UART):
        if UART:
            self.nsp32 = NSP32(PinRst, PinReady, DataChannelEnum.Uart, uartPotName = '/dev/ttyTHS1')
        else:
            self.nsp32 = NSP32(PinRst, PinReady, DataChannelEnum.Spi, spiBus = 0, spiDevice = 0)		# use SPI channel
        self.nsp32.Init()
        self.nsp32.Hello(0)
        self.nsp32.Standby(0)


    def read(self):

        self.nsp32.Wakeup()

        self.nsp32.GetWavelength(0)

        infoW = self.nsp32.GetReturnPacket().ExtractWavelengthInfo()


        # =============== spectrum acquisition ===============
        self.nsp32.AcqSpectrum(0, 32, 3, False)	# integration time = 32; frame avg num = 3; disable AE

        # "AcqSpectrum" command takes longer time to execute, the return packet is not immediately available
        # when the acquisition is done, a "ready trigger" will fire, and nsp32.GetReturnPacketSize() will be > 0	
        while self.nsp32.GetReturnPacketSize() <= 0 :
            self.nsp32.UpdateStatus()	# call UpdateStatus() to check async resul

        infoS = self.nsp32.GetReturnPacket().ExtractSpectrumInfo()

        self.nsp32.Standby(0)

        return infoW, infoS


class distanceSensor:
    def __init__(self):
        self.ToF = qwiic.QwiicVL53L1X()
        if (self.ToF.sensor_init() == None):					 # Begin returns 0 on a good init
            print("Sensor online!\n")

    def read(self):
        try:
            self.ToF.start_ranging()						 # Write configuration bytes to initiate measurement
            time.sleep(.005)
            distance = self.ToF.get_distance()	 # Get the result of the measurement from the sensor
            time.sleep(.005)
            self.ToF.stop_ranging()
            distanceInches = distance / 25.4
            distanceFeet = distanceInches / 12.0

            return distance, distanceFeet

        except Exception as e:
            print(e)


class OBJ_DETECTION():
    def __init__(self, model_path, classes):
        self.classes = classes
        self.yolo_model = attempt_load(weights=model_path)
        self.input_width = 320

    def detect(self,main_img):
        height, width = main_img.shape[:2]
        new_height = int((((self.input_width/width)*height)//32)*32)

        img = cv2.resize(main_img, (self.input_width,new_height))
        img = cv2.cvtColor(img,cv2.COLOR_BGR2RGB)
        img = np.moveaxis(img,-1,0)
        img = torch.from_numpy(img).to(device)
        img = img.float()/255.0  # 0 - 255 to 0.0 - 1.0
        if img.ndimension() == 3:
            img = img.unsqueeze(0)

        pred = self.yolo_model(img, augment=False)[0]
        pred = non_max_suppression(pred, conf_thres=0.25, iou_thres=0.45, classes=None)
        items = []
        
        if pred[0] is not None and len(pred):
            for p in pred[0]:
                score = np.round(p[4].cpu().detach().numpy(),2)
                label = self.classes[int(p[5])]
                xmin = int(p[0] * main_img.shape[1] /self.input_width)
                ymin = int(p[1] * main_img.shape[0] /new_height)
                xmax = int(p[2] * main_img.shape[1] /self.input_width)
                ymax = int(p[3] * main_img.shape[0] /new_height)

                item = {'label': label,
                        'bbox' : [(xmin,ymin),(xmax,ymax)],
                        'score': score
                        }

                items.append(item)

        return items


class Detector:

    def __init__(self):
        self.model = attempt_load(weights='yolov5n.pt')
        self.current_image = None
        self.annotated_image = None
        self.predictions = None
        self.input_width = 320

    def pred(self, image):
        self.current_image = image
        self.annotated_image = image


        height, width = image.shape[:2]
        new_height = int((((self.input_width/width)*height)//32)*32)

        img = cv2.resize(image, (self.input_width,new_height))

        img = cv2.cvtColor(img,cv2.COLOR_BGR2RGB)
        img = np.moveaxis(img,-1,0)

        img = torch.from_numpy(img).to(device)

        img = img.float()/255.0  # 0 - 255 to 0.0 - 1.0
        if img.ndimension() == 3:
            img = img.unsqueeze(0)


        self.predictions = self.model(self.img)

        for i in self.predictions.xyxy[0].cpu().numpy():
            self.annotated_image = cv2.rectangle(self.annotated_image, (int(i[0]), int(i[1])), (int(i[2]), int(i[3])), (0,255,0), 1)
    


class CSI_CAM:
    def __init__(self, gstream_pipe):
        self.gstream_pipe = gstream_pipe
        self.cap = cv2.VideoCapture(self.gstream_pipe, cv2.CAP_GSTREAMER)
        self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 0)

    def start(self):
        if not self.cap.isOpened():
            self.cap = cv2.VideoCapture(self.gstream_pipe, cv2.CAP_GSTREAMER)
            self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
    
    def read(self):
        #cap = cv2.VideoCapture(self.gstream_pipe)
        if self.cap.isOpened():
            self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
            _, frame = self.cap.read()
        #cap.release()
        return frame
    
    def end(self):
        if self.cap.isOpened():
            self.cap.release()


class CSI_Camera:

    def __init__(self):
        # Initialize instance variables
        # OpenCV video capture element
        self.video_capture = None
        # The last captured image from the camera
        self.frame = None
        self.grabbed = False
        # The thread where the video capture runs
        self.read_thread = None
        self.read_lock = threading.Lock()
        self.running = False

    def open(self, gstreamer_pipeline_string):
        try:
            self.video_capture = cv2.VideoCapture(
                gstreamer_pipeline_string, cv2.CAP_GSTREAMER
            )
            # Grab the first frame to start the video capturing
            self.grabbed, self.frame = self.video_capture.read()

        except RuntimeError:
            self.video_capture = None
            print("Unable to open camera")
            print("Pipeline: " + gstreamer_pipeline_string)


    def start(self):
        if self.running:
            print('Video capturing is already running')
            return None
        # create a thread to read the camera image
        if self.video_capture != None:
            self.running = True
            self.read_thread = threading.Thread(target=self.updateCamera)
            self.read_thread.start()
        return self

    def stop(self):
        self.running = False
        # Kill the thread
        self.read_thread.join()
        self.read_thread = None

    def updateCamera(self):
        # This is the thread to read images from the camera
        while self.running:
            try:
                grabbed, frame = self.video_capture.read()
                with self.read_lock:
                    self.grabbed = grabbed
                    self.frame = frame
            except RuntimeError:
                print("Could not read image from camera")
        # FIX ME - stop and cleanup thread
        # Something bad happened

    def read(self):
        with self.read_lock:
            frame = self.frame.copy()
            grabbed = self.grabbed
        return grabbed, frame

    def release(self):
        if self.video_capture != None:
            self.video_capture.release()
            self.video_capture = None
        # Now kill the thread
        if self.read_thread != None:
            self.read_thread.join()

def gstreamer_pipeline(
    sensor_id=0,
    capture_width=1920,
    capture_height=1080,
    display_width=1920,
    display_height=1080,
    framerate=30,
    flip_method=0,
):
    return (
        "nvarguscamerasrc sensor-id=%d ! "
        "video/x-raw(memory:NVMM), width=(int)%d, height=(int)%d, framerate=(fraction)%d/1 ! "
        "nvvidconv flip-method=%d ! "
        "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! appsink"
        % (
            sensor_id,
            capture_width,
            capture_height,
            framerate,
            flip_method,
            display_width,
            display_height,
        )
    )
