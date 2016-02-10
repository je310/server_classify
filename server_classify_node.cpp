#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <ros/ros.h>
#include <iostream>
#include <thread>         // std::thread
#include "std_msgs/Float32MultiArray.h"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
//cn24 includes
#include <fstream>
#include <cstring>
#include <sstream>

using namespace cv;
using namespace std;

int gtk_false = 0;
int cardInUse = 0;
int newImageReady = 0;
int cardInUse2 = 0;
int classificationComplete = 0;


String TempLocation = "~/catkin_ws/src/server_classify/temp/";
String TempJPG      = "image.jpg";

//cn24 file locations.
std::string output_image_fname  = "/Users/josh/catkin_ws/src/server_classify/temp/output.jpg";
std::string output_image_fname2  = "/Users/josh/catkin_ws/src/server_classify/temp/output2.jpg";
std::string input_image_fname   = "/Users/josh/catkin_ws/src/server_classify/temp/image.jpg";
std::string input_image_fname2   = "/Users/josh/catkin_ws/src/server_classify/temp/image2.jpg";
std::string param_tensor_fname  = "~/Card1/build/epoch_5.Tensor"; //trained .Tensor
std::string net_config_fname    = "~/Card1/build/labelmefacade.net";     // .net file
std::string dataset_config_fname= "~/Card1/build/dataset_config3Cat.set"; // .set file
std::string command =             "~/Card1/build/classifyImage";
std::string command2=  		"~/Card2/build/classifyImage";

Mat segImage, segImage2, image_in;
ros::Time imageTime;
ros::Time imageTimeUsed;
ros::Time image1Time;
ros::Time image2Time;
ros::Time segTime;
image_transport::Publisher image_out_pub;
cv_bridge::CvImage out_msg;
void image_Cb(const sensor_msgs::ImageConstPtr& msg)
{

	//get image from message.
	cv_bridge::CvImagePtr cv_ptr;
	try
	{
		cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
	}
	catch (cv_bridge::Exception& e)
	{
		ROS_ERROR("cv_bridge exception: %s", e.what());
		return;
	}
	image_in = cv_ptr->image;
	imageTime = msg->header.stamp; 
	newImageReady = 1;
}

void classifyImage(std::string command){
	cardInUse = 1;
	int i = system(command.c_str());
	segImage=imread(output_image_fname);
	classificationComplete = 1;
	segTime = image1Time;
    classificationComplete = 0;
	cv::resize(segImage, segImage , cv::Size(),0.25,0.25, cv::INTER_LINEAR);
    out_msg.image = segImage;
    out_msg.header.stamp = segTime;
    image_out_pub.publish(out_msg.toImageMsg());
	cardInUse = 0;
}

void classifyImage2(std::string command){
	cardInUse2 = 1;
	int i = system(command.c_str());
	segImage2=imread(output_image_fname2);
	classificationComplete = 1;
	segTime = image2Time;
	cv::resize(segImage2, segImage2 , cv::Size(),0.25,0.25, cv::INTER_LINEAR);
    out_msg.image = segImage2;
    out_msg.header.stamp = segTime;
    image_out_pub.publish(out_msg.toImageMsg());
	cardInUse2 = 0;
}


int main(int argc, char** argv)
{

	std::string fullCommand =   command             + " " +
		dataset_config_fname+ " " +
		net_config_fname    + " " +
		param_tensor_fname  + " " +
		input_image_fname   + " " +
		output_image_fname  + " > /dev/null";
	std::string fullCommand2 =   command2            + " " +
		dataset_config_fname+ " " +
		net_config_fname    + " " +
		param_tensor_fname  + " " +
		input_image_fname2   + " " +
		output_image_fname2  + " > /dev/null";

	ros::init(argc, argv, "cn24ROS_node");
	ros::NodeHandle nh_;
	image_transport::ImageTransport it_(nh_);
	cout<<"starting cn24ROS_node"<<endl;
	 ROS_INFO("starting server identification \n");

	image_out_pub = it_.advertise("/server_classify/image_out",1);
	image_transport::Subscriber image_in_pub = it_.subscribe("/camera_image_slow",1,image_Cb);


	
	out_msg.encoding = sensor_msgs::image_encodings::BGR8;
	while(ros::ok()){
		ros::spinOnce();

		if((cardInUse == 0 ||cardInUse2 ==0) && newImageReady ==1){
			newImageReady = 0;
			if(cardInUse ==0){
				image1Time = imageTime;
				imwrite(input_image_fname,image_in);
				ROS_INFO("using card 1 using %s",fullCommand.c_str());
				std::thread classThread(classifyImage,fullCommand);
				classThread.detach();
			}
			else{
				image2Time = imageTime;
				imwrite(input_image_fname2,image_in);
				ROS_INFO("using card 2 using %s",fullCommand2.c_str());
				std::thread classThread2(classifyImage2,fullCommand2);
				classThread2.detach();
			}
		}			
	}					
	return 0;
}
