#compile code
#g++ SingleFrameCaptureRealSense.cpp -o output -lGLU -lglut -lopencv_core -lrealsense2 -lopencv_imgproc -lopencv_imgcodecs
# gather weather data
echo "Capturing Weather Data"
python /home/nvidia/Documents/sentinel_home/NewBMECode.py

# gather camera data
./output

# mount usb if exists
echo "Mounting USB device if it exists"
sudo mount /dev/sda1 /USB_Transfer

# copy files to usb
echo "Copying files to USB device if present and mounted"
cp -a /home/nvidia/Documents/sentinel_home/Weather/WeatherInformation /USB_Transfer
cp -a /home/nvidia/Documents/sentinel_home/PlantPictures/TakenImages /USB_Transfer
