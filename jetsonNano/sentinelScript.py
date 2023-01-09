import cv2
import threading
import numpy as np
import torch
from sentinel_utils import CSI_Camera, gstreamer_pipeline, Detector, CSI_CAM

import qwiic_vl53l1x as qwiic
import time


# try initializing the cameras and taking pictures

cam = CSI_CAM(gstream_pipe=    gstreamer_pipeline(
        sensor_id=1,
        capture_width=1920,
        capture_height=1080,
        flip_method=0,
        display_width=960,
        display_height=540,
    ))

cam2 = CSI_CAM(gstream_pipe=    gstreamer_pipeline(
        sensor_id=0,
        capture_width=1920,
        capture_height=1080,
        flip_method=0,
        display_width=960,
        display_height=540,
    ))


while True:

    image = cam.read()

    cv2.imshow("pred", image)

    #cv2.waitKey(0)

    im2 = cam2.read()

    cv2.imshow("pred2", im2 )
    cv2.waitKey(0)

keyCode = cv2.waitKey(0)
# Stop the program on the ESC key
