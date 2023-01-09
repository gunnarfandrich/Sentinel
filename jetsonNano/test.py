import cv2
import threading
import numpy as np
from sentinel_utils import CSI_Camera, temp_sensor, gstreamer_pipeline, Detector, OBJ_DETECTION, distanceSensor, PicoPins, CSI_CAM, RepeatTimer, spectrumSensor
from models.experimental import attempt_load
import Jetson.GPIO as GPIO

MAX_HEIGHT = 3 #max height
NUM_PHOTOS = 3 #number of photos to take and save
MARGIN_OF_ERROR_HEIGHT = 0.20


mainLoopLock = threading.Lock()
global going_up
going_up = True

photo_ranges = [(x- MARGIN_OF_ERROR_HEIGHT, x + MARGIN_OF_ERROR_HEIGHT) for x in range(0,MAX_HEIGHT,int(MAX_HEIGHT/NUM_PHOTOS))]


photo_ranges = [(1.2, 1.6), (2.2, 2.6), (2.8, 3.2)]
#print(photo_ranges)


cam1 = CSI_CAM(gstream_pipe=    gstreamer_pipeline(
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



temp = temp_sensor(save_dir= 'sensorInfo/')
sensor = distanceSensor()
print("Distance Sensor Online")
#sensor_spectrum = spectrumSensor(UART=True)
print("Spectrum Sensor Online ")
pins = PicoPins(lock = mainLoopLock, Pulse = False)

def checkHeight():
    global going_up

    mainLoopLock.acquire()
    _, distanceFeet = sensor.read() #Read the Height Sensor

    print(" Distance(ft): %s" % ( distanceFeet))

    if not going_up and distanceFeet < 1.5:
        print("Done, Predicting on Photos Now")
        pins.pins_stop()
        mainloop.cancel()
        mainLoopLock.release()
        return 0

    elif not going_up:
        print("Going down, at " + str(distanceFeet) + "currently")
        pins.pins_down()


    if (distanceFeet > MAX_HEIGHT or len(photo_ranges) == 0) and going_up :
        print("Going Down Now")
        
        pins.pins_stop()
        pins.pins_down()

        going_up = False
        print("All photos taken, Sentinel Retracting")

    elif going_up:
        for i in photo_ranges: # check if it is in the correct ranges
            if (i[0] <= distanceFeet < i[1]): # check if going up ## NEED TO ADD

                print("Taking a photo at " + str(distanceFeet) + " FT")

                pins.pins_stop() # stop going up 
                
                im = cam1.read() # Read Cameras
                im2 = cam2.read()
                
                temp.read()
                temp.save(str(int(distanceFeet)) + 'FT_sensor.txt')

                cv2.imwrite('rawPhotos/' +  str(int(distanceFeet)) + 'FT_photo1.jpg', im )
                cv2.imwrite('rawPhotos/' + str(int(distanceFeet)) + 'FT_photo2.jpg', im2 )
                
                photo_ranges.remove(i)


                pins.pins_up()
                break

    mainLoopLock.release()


mainloop = RepeatTimer(1 , checkHeight)
mainloop.start()





