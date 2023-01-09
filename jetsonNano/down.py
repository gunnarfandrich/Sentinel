import Jetson.GPIO as GPIO
GPIO.setmode(GPIO.BOARD)
GPIO.setup(35, GPIO.OUT)
GPIO.output(35, GPIO.HIGH)

