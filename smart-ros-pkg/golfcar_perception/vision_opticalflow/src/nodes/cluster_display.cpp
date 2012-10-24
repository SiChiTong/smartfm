/**
 * Displays cluster. Subscribes to "cluster" and "image" topics.
*/

#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <message_filters/time_synchronizer.h>
#include <opencv2/highgui/highgui.hpp>

#include <vision_opticalflow/Feature.h>
#include <vision_opticalflow/Cluster.h>
#include <vision_opticalflow/Clusters.h>

class ClusterDisplayNode
{
public:
    ClusterDisplayNode();
    void spin();
private:
    ros::NodeHandle nh_;
    image_transport::ImageTransport it_;
    image_transport::SubscriberFilter frame_sub_;
    message_filters::Subscriber<vision_opticalflow::Clusters> clusters_sub_;

    typedef message_filters::sync_policies::ApproximateTime<
    sensor_msgs::Image, vision_opticalflow::Clusters
    > MySyncPolicy;
    message_filters::Synchronizer<MySyncPolicy> synchronizer;

    std::string displayWindowName_;

    void displayCallback(const sensor_msgs::Image::ConstPtr & frame,
        const vision_opticalflow::Clusters::ConstPtr & clusters_msg);
};

ClusterDisplayNode::ClusterDisplayNode()
: it_(nh_),
  frame_sub_(it_, "image", 20),
  clusters_sub_(nh_, "clusters", 20),
  synchronizer(MySyncPolicy(20), frame_sub_, clusters_sub_),
  displayWindowName_("")
{
    synchronizer.registerCallback( boost::bind(&ClusterDisplayNode::displayCallback, this, _1, _2) );
}

void ClusterDisplayNode::displayCallback(const sensor_msgs::Image::ConstPtr & frame,
        const vision_opticalflow::Clusters::ConstPtr & clusters_msg)
{
    cv::Point2f cluster_pos;
   
    cv::Scalar color[] = {cv::Scalar(255,0,0), cv::Scalar(125,0,0), cv::Scalar(0,255,0), cv::Scalar(0,125,0), cv::Scalar(0,0,255), cv::Scalar(0,0,125),
    cv::Scalar(255,255,0), cv::Scalar(125,125,0), cv::Scalar(255,0,255), cv::Scalar(125,0,125), cv::Scalar(0,255,255), cv::Scalar(0,125,125) };
    
//     std::cout << " displayCallback " << std::endl;
    
    if( displayWindowName_.length()==0 ) {
        displayWindowName_ = clusters_sub_.getTopic();
        cv::namedWindow(displayWindowName_, CV_WINDOW_NORMAL);
    }
    cv_bridge::CvImageConstPtr cvImgFrame = cv_bridge::toCvCopy(frame, "bgr8");
    cv::Mat img = cvImgFrame->image;

    for( unsigned i=0; i< clusters_msg->clusters_info.size(); i++ )
    {
        std::vector<float> xpos,ypos;
        float x_max,x_min;
        float y_max,y_min;
        
//         for(unsigned j=0; j<clusters_msg->clusters_info[i].members.size(); j++)
//         {
//             xpos.push_back(clusters_msg->clusters_info[i].members[j].x);
//             ypos.push_back(clusters_msg->clusters_info[i].members[j].y);
//         }
//         x_max = *std::max_element(xpos.begin(),xpos.end());     x_min = *std::min_element(xpos.begin(),xpos.end());
//         y_max = *std::max_element(ypos.begin(),ypos.end());     y_min = *std::min_element(ypos.begin(),ypos.end());

//         std::cout << "x_max : " << x_max << " x_min : "<< x_min << " y_max : " << y_max << " y_min : " << y_min <<std::endl;
        
        cluster_pos.x = clusters_msg->clusters_info[i].centroid.x;
        cluster_pos.y = clusters_msg->clusters_info[i].centroid.y;
        cv::circle(img, cluster_pos, 2, color[clusters_msg->clusters_info[i].id%12]); //[clusters_msg->clusters_info[i].id%12
//         cv::rectangle(img, cv::Point(x_min,y_max), cv::Point(x_max,y_min), color[clusters_msg->clusters_info[i].id%12]);
//         std::cout << "x_cen_vel : " << clusters_msg->clusters_info[i].centroid_vel.x << " y_cen_vel : "<< clusters_msg->clusters_info[i].centroid_vel.y <<std::endl;
    }
//     std::cout << " ------------------------ " <<std::endl;
    cv::imshow(displayWindowName_, img);
}

void ClusterDisplayNode::spin()
{
    while( ros::ok() )
    {
        if( displayWindowName_.length()==0 )
            ros::Duration(0.1).sleep();
        else
            if( cv::waitKey(10)=='q' )
                return;
        ros::spinOnce();
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "cluster_display", ros::init_options::AnonymousName);
    ClusterDisplayNode node;
    node.spin();
    return 0;
}