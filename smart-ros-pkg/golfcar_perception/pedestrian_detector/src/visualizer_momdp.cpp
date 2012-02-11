#include "visualizer_momdp.h"
//#include "cv_helper.h"
#include <iomanip>
#include <iostream>

using namespace cv;
using namespace std;

HOGVisualizer::HOGVisualizer(ros::NodeHandle &n) : n_(n), it_(n_)
{
    ros::NodeHandle nh("~");
    nh.param("ROI_text", ROI_text, true);
    nh.param("ROI_rect", ROI_rect, false);
    nh.param("Publish_verified", publish_verified, true);
    /// get image from the USB cam
    image_sub_.subscribe(it_,"/usb_cam/image_raw",20);

    /// start processign only after the first image is present
    started = false;

    /// from laser (green rect)
    people_roi_sub_.subscribe(n, "veri_pd_vision", 20);
    ped_bel_sub_ = n.subscribe("peds_believes", 1, &HOGVisualizer::pedBeliefCallback, this);
    typedef sync_policies::ApproximateTime<sensor_msgs::Image, pedestrian_vision_batch> MySyncPolicy;
    Synchronizer<MySyncPolicy> sync(MySyncPolicy(20), image_sub_, people_roi_sub_);
    sync.registerCallback(boost::bind(&HOGVisualizer::peopleCallback,this, _1, _2));

    cvNamedWindow("MOMDP Visualizer");

    ros::spin();
}

HOGVisualizer::~HOGVisualizer(){}

void HOGVisualizer::peopleCallback(const sensor_msgs::ImageConstPtr image, const pedestrian_vision_batchConstPtr vision_roi)
{
    Mat img;
    Cv_helper::sensormsgsToCv(image, img);

    /// roi_rects_ : laser based ( blue )
    /// detect_rects_ : vision based detection ( green )
    /// verified_rects_ : final confirmation  (red )


    for(unsigned int i=0;i<vision_roi->pd_vector.size();i++)
    {
        Point UL = Point(vision_roi->pd_vector[i].cvRect_x1, vision_roi->pd_vector[i].cvRect_y1);
        Point BR = Point(vision_roi->pd_vector[i].cvRect_x2, vision_roi->pd_vector[i].cvRect_y2);
        sensing_on_road::pedestrian_vision temp_rect = vision_roi->pd_vector[i];

        if(ROI_rect)
        {
            if(publish_verified)
            {
                if(vision_roi->pd_vector[i].decision_flag)
                    rectangle(img,UL, BR, Scalar(255,0,0), 1);
            }
            else rectangle(img,UL, BR, Scalar(255,0,0), 1);

        }
        if(ROI_text) drawIDandConfidence(img, temp_rect);

    }
    started = true;
    imshow("MOMDP Visualizer", img);
    cvWaitKey(3);
}

void HOGVisualizer::pedBeliefCallback(ped_momdp_sarsop::peds_believes ped_bel_cb)
{
    ped_bel.resize(ped_bel_cb.believes.size());
    for(size_t i=0; i<ped_bel_cb.believes.size(); i++)
    {
        double left =  ped_bel_cb.believes[i].belief_value[0] + ped_bel_cb.believes[i].belief_value[3];
        double right =  ped_bel_cb.believes[i].belief_value[1] + ped_bel_cb.believes[i].belief_value[2];
        int decision;

        if(ped_bel_cb.believes[i].action==1)
            decision=1;
        else if(ped_bel_cb.believes[i].action==2)
            decision=-1;
        else if(ped_bel_cb.believes[i].action==0)
        {
            if(ped_bel_cb.robotv>0.1)
                decision=1;
            else if( ped_bel_cb.robotv < 0.1)
                decision=-1;
        }

        ped_bel[i].decision = decision;
        ped_bel[i].left_side = left;
        ped_bel[i].right_side = right;
        ped_bel[i].id = ped_bel_cb.believes[i].ped_id;
    }

}

void HOGVisualizer::drawIDandConfidence(Mat& img, sensing_on_road::pedestrian_vision& pv)
{
    std::stringstream ss,ss2;
    float offset = 10.0;
    ss<<pv.object_label;
    /// first row
    Point UL = Point(pv.cvRect_x1, pv.cvRect_y1); /// top left
    Point UR = Point(pv.cvRect_x2, pv.cvRect_y1); /// top left
    Point UR_UP = Point(pv.cvRect_x2, pv.cvRect_y1-offset); /// top left
    Point UC = Point((pv.cvRect_x2+pv.cvRect_x1)/2, pv.cvRect_y1); /// top mid
    Point UP_C_OFF = Point((pv.cvRect_x1+pv.cvRect_x2)/2, pv.cvRect_y1+offset); /// top thickness
    Point UL_OFF = Point(pv.cvRect_x1, pv.cvRect_y1+offset); /// topleft thickness
    //Point UR_OFF = Point(pv.cvRect_x2, pv.cvRect_y1+offset); /// topright thickness
    
    /// second row
    Point UR_2OFF = Point(pv.cvRect_x2, pv.cvRect_y1+(2*offset)); /// 2nd row mid thickness
    Point UR_OFF = Point(pv.cvRect_x2, pv.cvRect_y1+offset); 
    Point BR = Point(pv.cvRect_x2, pv.cvRect_y2); /// bot left
    Point BL = Point(pv.cvRect_x1, pv.cvRect_y2); /// bot right
    
    for(size_t i=0; i<ped_bel.size(); i++)
    {
		
        if(ped_bel[i].id == pv.object_label)
        {
            cout<<ped_bel[i].id<<" "<<ped_bel[i].left_side<<" "<<ped_bel[i].right_side<<endl;
            
            //cout << " x " << pv.cvRect_x1 << " y " << pv.cvRect_y1; 
            //cout << " x " << pv.cvRect_x2 << " y " << pv.cvRect_y2 << endl;; 
            
			/// Gradient Coding
            //int left_gradient = 255-(255.0*ped_bel[i].left_side);
            //int right_gradient = 255-(255.0*ped_bel[i].right_side);

            //rectangle(img,UL, UP_C_OFF, Scalar(255,left_gradient,left_gradient), CV_FILLED);
            //rectangle(img,UL, UP_C_OFF, Scalar(255,left_gradient,left_gradient), CV_FILLED);
            //rectangle(img,UC, UR_OFF, Scalar(255,right_gradient,right_gradient), CV_FILLED);
            
            /// size encoding
            //rectangle(img,UL, UP_C_OFF, Scalar(255,255,255), CV_FILLED);
            /// fill with white
            //rectangle(img,UL, UR_OFF, Scalar(255,255,255), CV_FILLED);
            /// left bar
            double length = 40;//(pv.cvRect_x2 - pv.cvRect_x1);
            //Point leftP = Point(pv.cvRect_x1, -ped_bel[i].left_side*length+ pv.cvRect_y1);
            Point leftP = Point(pv.cvRect_x1, ped_bel[i].left_side*length+ pv.cvRect_y1);
            rectangle(img,leftP, UC, Scalar(255,0,0), CV_FILLED);
			//Point leftP2 = Point(pv.cvRect_x2, -30+ pv.cvRect_y1);
            //rectangle(img,leftP2, UC, Scalar(255,255,255), CV_FILLED);

            
            /// right bar
            Point rightP = Point(pv.cvRect_x2,  ped_bel[i].right_side*length + pv.cvRect_y1);
            rectangle(img, rightP, UC, Scalar(255,0,0), CV_FILLED);
            
            
            
            
            
            /// lower panel
            if(ped_bel[i].decision==-1) 
				rectangle(img,UL, UR_UP, Scalar(0,0,255), CV_FILLED); /// red
            else 
				rectangle(img,UL, UR_UP, Scalar(0,255,0), CV_FILLED); /// green
            
            
            
        }
    }

    putText(img, ss.str(), BL+Point(2,-2), FONT_HERSHEY_PLAIN, 0.8, cvScalar(0,255,255), 1, 8);
    //ss2<<setprecision(2)<<fixed<<pv.confidence*100.0;
    //putText(img, ss2.str(), BR+Point(-45,-2), FONT_HERSHEY_PLAIN, 0.8, cvScalar(0,255,255), 1, 8);
}
int main(int argc, char** argv)
{
    ros::init(argc, argv, "MOMDP_Visualizer");
    ros::NodeHandle n;
    HOGVisualizer ic(n);
    ros::spin();
    return 0;
}
