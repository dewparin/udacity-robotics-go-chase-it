#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

enum direction
{
    left,
    middle,
    right
};

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ROS_INFO("Driving the robot with lin_x: %1.2f, ang_z: %1.2f", lin_x, ang_z);

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    const int left_bound_index = img.step / 3;
    const int right_bound_index = left_bound_index * 2;
    // ROS_INFO("Left bound: %d, right bound: %d", left_bound_index, right_bound_index);

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    bool found_ball = false;
    direction target_direction;
    auto length = img.height * img.step;
    for (int i = 0; i < length; i++)
    {
        if (img.data[i] == white_pixel)
        {
            found_ball = true;
            int col_index = i % img.step;
            // ROS_INFO("Found ball at col index: %d", col_index);
            if (col_index < left_bound_index)
            {
                target_direction = left;
            }
            else if (col_index >= left_bound_index &&
                     col_index <= right_bound_index)
            {
                target_direction = middle;
            }
            else
            {
                target_direction = right;
            }

            break;
        }
    }

    if (found_ball)
    {
        switch (target_direction)
        {
        case left:
            // ROS_INFO("Found ball at direction: left");
            drive_robot(0.0, 0.5);
            break;
        case middle:
            // ROS_INFO("Found ball at direction: middle");
            drive_robot(0.5, 0.0);
            break;
        case right:
            // ROS_INFO("Found ball at direction: right");
            drive_robot(0.0, -0.5);
            break;

        default:
            break;
        }
    }
    else
    {
        drive_robot(0.0, 0.0);
    }
}

int main(int argc, char **argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}