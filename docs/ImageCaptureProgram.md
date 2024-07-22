# Sentinel Image Capture Program

The following document details the experimental frame capture program developed from the information supplied from [Intel.](https://github.com/IntelRealSense/librealsense)

## Camera Initialization
Before capturing any frames, the D415 cameras must be initialized.

```c++
 // Configure depth and color streams for Camera 1
 rs2::config config_1;
 rs2::pipeline pipeline_1;
 config_1.enable_device("211622062823");
 config_1.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
 config_1.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_RGB8, 30);
 
 std::cout << "Camera 1 Pipeline Loaded \n";
    
 // Configure depth and color streams for Camera 2
 rs2::pipeline pipeline_2;
 rs2::config config_2;
 config_2.enable_device("211422060341");
 config_2.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
 config_2.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_RGB8, 30);
    
 std::cout << "Camera 2 Pipeline Loaded \n";
```

As seen in the code snippet above, to initialize the cameras, first a pre-made configuration is loaded as well as a pre-defined pipeline. The camera serial number is next entered to explicitly assign each camera to the camera's corresponding pipeline. Finally, the camera is assigned depth and color formats, as well as a stream resolution.

## Frame Pipeline

After configuring the cameras, the Jetson Nano will attempt to open the pipeline for each camera.

```c++
try
{
 pipeline_1.start(config_1);
 std::cout << "Camera 1 Stream Initialized \n";
 
 throw emptyString;
}

// start with the most specific handlers
catch (const rs2::camera_disconnected_error& e)
{
    std::cout << "Camera 1 not connected! Please connect it" << endl;
    // wait for connect event
}
// continue with more general cases
catch (const rs2::recoverable_error& e)
{
    std::cout << "Operation regarding camera 1 failed, please try again" << endl;
}
// you can also catch "anything else" raised from the library by catching rs2::error
catch (const rs2::error& e)
{
    std::cout << "Some other error occurred (camera1)!" << endl;
}

catch (...)
{
break;
}
```

When attempting to open the pipeline, if any errors occur, they are caught and output to users via the terminal.

## Configuring and Capturing Frames

Next, camera data is configured and video frames are captured.

```c++
std::cout << "Cameras were successfully detected, capturing images now. \n";

rs2::align align_to_C(RS2_STREAM_COLOR);
rs2::align align_to_D(RS2_STREAM_DEPTH);

// Camera 1
rs2::frameset frames_1 = pipeline_1.wait_for_frames();
frames_1 = align_to_C.process(frames_1);
rs2::video_frame color_frame_1 = frames_1.get_color_frame();
frames_1= align_to_D.process(frames_1);
rs2::depth_frame depth_frame_1 = frames_1.get_depth_frame();


// Camera 2
rs2::frameset frames_2 = pipeline_2.wait_for_frames();
frames_2 = align_to_C.process(frames_2);
rs2::video_frame color_frame_2 = frames_2.get_color_frame();
frames_2= align_to_D.process(frames_2);
rs2::depth_frame depth_frame_2 = frames_2.get_depth_frame();
```

As seen in the code snipped above, data in each pipeline is first aligned to pre-defined stream color and depth configurations.

## Processing Frame Data

The program then waits for frames and upon their acquisition the frames are processed for color and depth information.

```c++
//Convert Camera 1 Frames to Usable Data type
Mat color_frame_1_mat;
Mat depth_frame_1_mat;
color_frame_1_mat = frame_to_mat(color_frame_1);
depth_frame_1_mat = frame_to_mat(depth_frame_1);

//applyColorMap requires 8UC3 or 8UC1 input image type
Mat depth_frame_1_mat_8UC3;
depth_frame_1_mat.convertTo(depth_frame_1_mat_8UC3,CV_8UC3);

//Get color mapped image
Mat depth_frame_1_CM;
applyColorMap(depth_frame_1_mat_8UC3, depth_frame_1_CM, cv::COLORMAP_JET);

//Convert Camera 2 Frames to Usable Data type
Mat color_frame_2_mat;
Mat depth_frame_2_mat;
color_frame_2_mat = frame_to_mat(color_frame_2);
depth_frame_2_mat = frame_to_mat(depth_frame_2);

//applyColorMap requires 8UC3 or 8UC1 input image type
Mat depth_frame_2_mat_8UC3;
depth_frame_2_mat.convertTo(depth_frame_2_mat_8UC3,CV_8UC3);

//Get color mapped image
Mat depth_frame_2_CM;
applyColorMap(depth_frame_2_mat_8UC3, depth_frame_2_CM, cv::COLORMAP_JET);
```

As seen in the code snippet above, matrices are first created to store color and depth information. The frames are then converted to matrices and stored in said storage matrices. The data is the depth matrices is then mapped to 8UC3 and the data in the color matrices has a color map applied to it.

## Exporting Frames to Local Storage

Finally, the program outputs the captured images with the corresponding capture time appended to the file name.
```c++
// Start gathering system time
auto start = std::chrono::system_clock::now();
// Some computation here
auto end = std::chrono::system_clock::now();

std::chrono::duration<double> elapsed_seconds = end-start;
std::time_t end_time = std::chrono::system_clock::to_time_t(end);

string strtime = std::ctime(&end_time);

string filename1 = "/home/nvidia/Documents/sentinel_home/PlantPictures/TakenImages/" + clean(strtime) + "c2116_color.jpg";
string filename2 = "/home/nvidia/Documents/sentinel_home/PlantPictures/TakenImages/" + clean(strtime) + "c2116_depth.jpg";
string filename3 = "/home/nvidia/Documents/sentinel_home/PlantPictures/TakenImages/" + clean(strtime) + "c2114_color.jpg";
string filename4 = "/home/nvidia/Documents/sentinel_home/PlantPictures/TakenImages/" + clean(strtime) + "c2114_depth.jpg";

std::cout << "Time is: "<< strtime;

cv::imwrite(filename1, color_frame_1_mat);
cv::imwrite(filename2, depth_frame_1_CM);
cv::imwrite(filename3, color_frame_2_mat);
cv::imwrite(filename4, depth_frame_2_CM);

std::cout << "Images successfully saved\n\n";
```

As seen in the code snippet, the program gathers the system time and outputs the captured images to the home directory of the Sentinel and two nested sub folders, "PlantPictures" and "TakenImages".
