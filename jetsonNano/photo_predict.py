import os 
import torch
import numpy as np
import cv2
from models.experimental import attempt_load
import Jetson.GPIO as GPIO
import pandas

WORK_PIN = 40
#GPIO.setmode(GPIO.BOARD)
#GPIO.setup(WORK_PIN, GPIO.OUT)



device = torch.device("cuda" if torch.cuda.is_available() else "cpu")


input_width = 320

model = torch.hub.load('../yolov5/', 'custom', path= 'yolov5n.pt', source = 'local')

model.cuda()

for filename in os.listdir('rawPhotos'):
    results = model('rawPhotos/' + filename)
    results.save(save_dir= 'annotatedPhotos/' + filename)    
    results.crop(save_dir = 'annotatedPhotos/' + filename)

#GPIO.output(WORK_PIN, GPIO.LOW)
