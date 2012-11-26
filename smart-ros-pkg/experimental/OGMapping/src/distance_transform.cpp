#include <ros/ros.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <vector>
#include <sensor_msgs/PointCloud.h>
#include <fmutil/fm_math.h>
#include <stdio.h>

//this implementation using OpenCV doesn't satisfy our purposes in road structure case;
//it is designed to build the voronoi graph for a space with discrete objects;
//not designed to build the voronoi graph inside a continuous object;
//please refer to CGAL for better implementation;

/*
using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
	 if(argc != 2){return -1;}

    Mat before = imread(argv[1], 0);

    Mat dist;
    Mat vonoroi;
    
    distanceTransform(before, dist, vonoroi, CV_DIST_L2, 3);

    imshow("before", before);
    imshow("non-normalized", dist);
    imshow("vonoroi", vonoroi);

    normalize(dist, dist, 0.0, 1.0, NORM_MINMAX);
    imshow("normalized", dist);
    waitKey();
    return 0;
}
*/

#include "cv.h"
#include "highgui.h"


char wndname[] = "Distance transform";
char tbarname[] = "Threshold";
int mask_size = CV_DIST_MASK_5;
int build_voronoi = 0;
int edge_thresh = 100;

// The output and temporary images
IplImage* dist = 0;
IplImage* dist8u1 = 0;
IplImage* dist8u2 = 0;
IplImage* dist8u = 0;
IplImage* dist32s = 0;

IplImage* gray = 0;
IplImage* edge = 0;
IplImage* labels = 0;

// threshold trackbar callback
void on_trackbar( int dummy )
{
    static const uchar colors[][3] = 
    {
        {0,0,0},
        {255,0,0},
        {255,128,0},
        {255,255,0},
        {0,255,0},
        {0,128,255},
        {0,255,255},
        {0,0,255},
        {255,0,255}
    };
    
    int msize = mask_size;

    cvThreshold( gray, edge, (float)edge_thresh, (float)edge_thresh, CV_THRESH_BINARY );

    if( build_voronoi )
        msize = CV_DIST_MASK_5;

    cvDistTransform( edge, dist, CV_DIST_L2, msize, NULL, build_voronoi ? labels : NULL );
	 
    if( !build_voronoi )
    {
        // begin "painting" the distance transform result
        cvConvertScale( dist, dist, 5000.0, 0 );
        cvPow( dist, dist, 0.5 );
    
        cvConvertScale( dist, dist32s, 1.0, 0.5 );
        cvAndS( dist32s, cvScalarAll(255), dist32s, 0 );
        cvConvertScale( dist32s, dist8u1, 1, 0 );
        cvConvertScale( dist32s, dist32s, -1, 0 );
        cvAddS( dist32s, cvScalarAll(255), dist32s, 0 );
        cvConvertScale( dist32s, dist8u2, 1, 0 );
        cvMerge( dist8u1, dist8u2, dist8u2, 0, dist8u );
        // end "painting" the distance transform result
    }
    else
    {
        int i, j;
        for( i = 0; i < labels->height; i++ )
        {
            int* ll = (int*)(labels->imageData + i*labels->widthStep);
            float* dd = (float*)(dist->imageData + i*dist->widthStep);
            uchar* d = (uchar*)(dist8u->imageData + i*dist8u->widthStep);
            for( j = 0; j < labels->width; j++ )
            {
                int idx = ll[j] == 0 || dd[j] == 0 ? 0 : (ll[j]-1)%8 + 1;
                int b = cvRound(colors[idx][0]);
                int g = cvRound(colors[idx][1]);
                int r = cvRound(colors[idx][2]);
                d[j*3] = (uchar)b;
                d[j*3+1] = (uchar)g;
                d[j*3+2] = (uchar)r;
            }
        }
    }
    
    cvShowImage( wndname, dist8u );
}

int main( int argc, char** argv )
{
    if(argc != 2){return -1;}

    if( (gray = cvLoadImage( argv[1], 0 )) == 0 )
        return -1;

    printf( "Hot keys: \n"
        "\tESC - quit the program\n"
        "\t3 - use 3x3 mask\n"
        "\t5 - use 5x5 mask\n"
        "\t0 - use precise distance transform\n"
        "\tv - switch Voronoi diagram mode on/off\n"
        "\tENTER - loop through all the modes\n" );

    dist = cvCreateImage( cvGetSize(gray), IPL_DEPTH_32F, 1 );
    dist8u1 = cvCloneImage( gray );
    dist8u2 = cvCloneImage( gray );
    dist8u = cvCreateImage( cvGetSize(gray), IPL_DEPTH_8U, 3 );
    dist32s = cvCreateImage( cvGetSize(gray), IPL_DEPTH_32S, 1 );
    edge = cvCloneImage( gray );
    labels = cvCreateImage( cvGetSize(gray), IPL_DEPTH_32S, 1 );

    cvNamedWindow( wndname, 1 );

    cvCreateTrackbar( tbarname, wndname, &edge_thresh, 255, on_trackbar );

    for(;;)
    {
        int c;
        
        // Call to update the view
        on_trackbar(0);

        c = cvWaitKey(0);

        if( (char)c == 27 )
            break;

        if( (char)c == '3' )
            mask_size = CV_DIST_MASK_3;
        else if( (char)c == '5' )
            mask_size = CV_DIST_MASK_5;
        else if( (char)c == '0' )
            mask_size = CV_DIST_MASK_PRECISE;
        else if( (char)c == 'v' )
            build_voronoi ^= 1;
        else if( (char)c == '\n' )
        {
            if( build_voronoi )
            {
                build_voronoi = 0;
                mask_size = CV_DIST_MASK_3;
            }
            else if( mask_size == CV_DIST_MASK_3 )
                mask_size = CV_DIST_MASK_5;
            else if( mask_size == CV_DIST_MASK_5 )
                mask_size = CV_DIST_MASK_PRECISE;
            else if( mask_size == CV_DIST_MASK_PRECISE )
                build_voronoi = 1;
        }
    }

    cvReleaseImage( &gray );
    cvReleaseImage( &edge );
    cvReleaseImage( &dist );
    cvReleaseImage( &dist8u );
    cvReleaseImage( &dist8u1 );
    cvReleaseImage( &dist8u2 );
    cvReleaseImage( &dist32s );
    cvReleaseImage( &labels );
    
    cvDestroyWindow( wndname );
    
    return 0;
}
