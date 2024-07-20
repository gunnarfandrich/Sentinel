#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/cv-helpers.hpp>
#include <ctime>

const size_t windowWidth = 1280;
const size_t windowHeight = 720;

bool tryFlag = 1;

std::string errorString = "Oops! Check that the cameras are correctly connected";
std::string emptyString = "";

using namespace cv;
using namespace rs2;
using namespace std;


std::string clean(std::string text)
{
    std::replace(text.begin(), text.end(), ' ', '_');
    std::replace(text.begin(), text.end(), '\n', '_');
    std::replace(text.begin(), text.end(), ':', '_');
    return text;
}

int main() {

 std::cout << "Intel Realsense Capture Program\nIot4Ag | University of Florida\nSentinel Mini, Spring 2024\n\n";

 std::cout << "Start! \n";
 
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
 



 // Start streaming from both cameras
 
 while (tryFlag)
 {
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
 }
 
 while (tryFlag)
 {
 	 try
	 {
		 pipeline_2.start(config_2);
 		 std::cout << "Camera 2 Stream Initialized \n";
		 
		 throw emptyString;
	 }
	 
	 // start with the most specific handlers
	 catch (const rs2::camera_disconnected_error& e)
	 {
	    	std::cout << "Camera 2 not connected! Please connect it" << endl;
	    	// wait for connect event
	 }
	 // continue with more general cases
	 catch (const rs2::recoverable_error& e)
	 {
	    	std::cout << "Operation regarding camera 2 failed, please try again" << endl;
	 }
	 // you can also catch "anything else" raised from the library by catching rs2::error
	 catch (const rs2::error& e)
	 {
	    	std::cout << "Some other error occurred (camera2) !" << endl;
	 }
	 
	 catch (...)
	 {
	 	break;
	 }
 }
 
 
 std::cout << "Cameras were successfully detected, capturing images now. \n";

 rs2::align align_to_C(RS2_STREAM_COLOR);
 rs2::align align_to_D(RS2_STREAM_DEPTH);

 // namedWindow("Tutorial Hello", WINDOW_AUTOSIZE);


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

  
  //Show all Streams in 4 popup windows
  //imshow("Color Stream 1", color_frame_1_mat);
  //imshow("Depth Stream 1", depth_frame_1_CM);
  //imshow("Color Stream 2", color_frame_2_mat);
  //imshow("Depth Stream 2", depth_frame_2_CM);
  
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

}
