/*
 * pcl_clustering.cpp
 *
 *  Created on: Jan 15, 2012
 *      Author: golfcar
 */
#include <ros/ros.h>

#include <pcl/ModelCoefficients.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/features/normal_3d.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/common/common.h>
#include <sensor_msgs/PointCloud.h>
#include <sensor_msgs/PointCloud2.h>
#include <fmutil/fm_math.h>
#include <std_msgs/Int64.h>

#include <sensor_msgs/LaserScan.h>
#include <sensor_msgs/point_cloud_conversion.h>
#include <laser_geometry/laser_geometry.h>
#include <tf/message_filter.h>
#include <message_filters/subscriber.h>

using namespace std;

class ped_clustering
{
public:
    ped_clustering();
private:
    void scanCallback(const sensor_msgs::PointCloud2ConstPtr& pc2);
    void clustering(const sensor_msgs::PointCloud2& pc2);
    void filterLines(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered);
    void extractCluster(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered, std::vector<pcl::PointIndices>& cluster_indices);
    void laserCallback(const sensor_msgs::LaserScanConstPtr& scan_in);
    ros::Subscriber cloud_sub_;
    message_filters::Subscriber<sensor_msgs::LaserScan> laser_sub_;
    ros::Publisher cloud_pub_;
    ros::Publisher poi_pub_, line_filtered_pub_;
    ros::Publisher ped_candNo_pub_;

    tf::TransformListener tf_;
    tf::MessageFilter<sensor_msgs::LaserScan> * tf_filter_;

    laser_geometry::LaserProjection projector_;
    string laser_frame_id_;
};

ped_clustering::ped_clustering()
{
    ros::NodeHandle nh;
    cloud_sub_ = nh.subscribe("sickldmrs/cloud2", 10, &ped_clustering::scanCallback, this);
    cloud_pub_ = nh.advertise<sensor_msgs::PointCloud>("pedestrian_cluster", 1);
    poi_pub_= nh.advertise<sensor_msgs::PointCloud>("pedestrian_poi",1);
    line_filtered_pub_ = nh.advertise<sensor_msgs::PointCloud2>("line_filtered",1);
    ped_candNo_pub_ = nh.advertise<std_msgs::Int64>("ped_candNo",1);
    laser_frame_id_ = "ldmrs0";
    laser_sub_.subscribe(nh, "sickldmrs/scan0", 10);
    tf_filter_ = new tf::MessageFilter<sensor_msgs::LaserScan>(laser_sub_, tf_, laser_frame_id_, 10);
    tf_filter_->registerCallback(boost::bind(&ped_clustering::laserCallback, this, _1));
    tf_filter_->setTolerance(ros::Duration(0.05));
    ros::spin();
}

void ped_clustering::scanCallback(const sensor_msgs::PointCloud2ConstPtr& pc2)
{

    clustering(*pc2);
}

inline geometry_msgs::Point32 getCenterPoint(geometry_msgs::Point32 start_pt, geometry_msgs::Point32 end_pt)
{
    geometry_msgs::Point32 center;
    center.x = (start_pt.x + end_pt.x)/2;
    center.y = (start_pt.y + end_pt.y)/2;
    center.z = (start_pt.z + end_pt.z)/2;
    return center;
}
void ped_clustering::laserCallback(const sensor_msgs::LaserScanConstPtr& scan_in)
{
    sensor_msgs::PointCloud pc;
    try{projector_.transformLaserScanToPointCloud(laser_frame_id_, *scan_in, pc, tf_);}
            catch (tf::TransformException& e){ROS_INFO_STREAM(e.what());return;}
    /* Tried with min_size=1, tolerance=1.5, the result is not encouraging. It shows a different
     * cluster each with single point although there are spearted by less than 0.5 m
     * sensor_msgs::PointCloud2 pc2;

    sensor_msgs::convertPointCloudToPointCloud2(pc,pc2);
    clustering(pc2);
    */
    /* Todo: add points into those known centroid*/
    double diff_t = 1.0;
    double seconddiff_t = 1.0;
    double z = 0;
    pc.points[0].z = 0;
    sensor_msgs::PointCloud centroid;
    centroid.header = pc.header;
    geometry_msgs::Point32 start_pt, center_pt;
    start_pt = pc.points[0];
    for(int i=1; i<pc.points.size(); i++)
    {
        if(fmutil::distance(pc.points[i-1].x, pc.points[i-1].y, pc.points[i].x, pc.points[i].y)>diff_t)
        {
            centroid.points.push_back(getCenterPoint(pc.points[i-1],start_pt));

            start_pt = pc.points[i];
            z++;
        }
        pc.points[i].z=z;
    }
    centroid.points.push_back(getCenterPoint(pc.points[pc.points.size()-1],start_pt));

    //compare the centroids, join them if there are near together
    //for now it will just look through exhaustively, more efficient method e.g. kNN search can be implemented
    sensor_msgs::PointCloud centroid2;
    centroid2.header = centroid.header;
    start_pt = centroid.points[0];
    //ROS_INFO("1st pass: %d", centroid.points.size());
    cloud_pub_.publish(centroid);
    vector<geometry_msgs::Point32>::iterator it;
    int first_loop = 0;
    for( it=centroid.points.begin(); it<centroid.points.end()-1; it++)
    {
        int second_loop = 0;
        cout<<"1L: "<<first_loop++<<endl;
        vector<geometry_msgs::Point32>::iterator it2;
        for(it2=it+1; it2<centroid.points.end(); it2++)
        {
            cout<<"2L: ";
            cout<<second_loop++<<endl;
            if(fmutil::distance(it->x, it->y, it2->x, it2->y)<seconddiff_t)
            {
                cout<<"Erasing "<<second_loop<<endl;
                geometry_msgs::Point32 cp = getCenterPoint(*it,*it2);
                it->x = cp.x; it->y = cp.y; it->z = cp.z;
                centroid.points.erase(it2);
            }
        }

    }
    poi_pub_.publish(centroid);

}

void ped_clustering::clustering(const sensor_msgs::PointCloud2 &pc)
{
    // Create the filtering object: downsample the dataset using a leaf size of 1cm

    pcl::PointCloud<pcl::PointXYZ> cloud_temp;
    pcl::fromROSMsg(pc, cloud_temp);
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered = cloud_temp.makeShared();
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_cluster(new pcl::PointCloud<pcl::PointXYZ>);
    int i = 0;

    //line filtered quite nicely, but then the side effect is the points become more cluttered

    std::vector<pcl::PointIndices> cluster_indices;
    extractCluster(cloud_filtered, cluster_indices);
    //cloud_cluster = (new pcl::PointCloud<pcl::PointXYZ>);
    for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin (); it != cluster_indices.end (); ++it)
    {
        for (std::vector<int>::const_iterator pit = it->indices.begin (); pit != it->indices.end (); pit++)
            cloud_cluster->points.push_back (cloud_filtered->points[*pit]);
    }


    //now line filters
    filterLines(cloud_cluster);
    sensor_msgs::PointCloud2 line_filtered;
    pcl::toROSMsg(*cloud_cluster, line_filtered);
    line_filtered.header = pc.header;
    line_filtered_pub_.publish(line_filtered);

    //then cluster again
    cluster_indices.clear();
    extractCluster(cloud_cluster, cluster_indices);
    sensor_msgs::PointCloud total_clusters, ped_poi;
    int bounded_filter = 0;

    for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin (); it != cluster_indices.end (); ++it)
    {
        /**/
        Eigen::Vector4f min_pt, max_pt,mid_pt, abs_distance;
        pcl::getMinMax3D(*cloud_cluster, it->indices, min_pt, max_pt);
        //cout<<i<<" indices min_pt: "<<min_pt<<" max_pt: "<<max_pt<<endl;
        abs_distance = max_pt - min_pt;
        mid_pt = (max_pt + min_pt)/2.0;

        bool max_dist = fmutil::distance(mid_pt[0], mid_pt[1], 0, 0)<20;
        //just take all the clusters
        if(abs_distance[0]< 1.5 && abs_distance[1]<1.5 && abs_distance[2]>0.1 && max_dist)
        {
            geometry_msgs::Point32 p;
            p.x = mid_pt[0];
            p.y = mid_pt[1];
            p.z = mid_pt[2];
            ped_poi.points.push_back(p);
            for (std::vector<int>::const_iterator pit = it->indices.begin (); pit != it->indices.end (); pit++)
            {

                p.x = cloud_cluster->points[*pit].x;
                p.y = cloud_cluster->points[*pit].y;
                p.z = i;
                total_clusters.points.push_back(p);
            }

            bounded_filter++;
        }

        i++;
    }
    cout<<"After bounding box filtering = "<<bounded_filter<<endl;
    total_clusters.header = pc.header;
    ped_poi.header = pc.header;
    cloud_pub_.publish(total_clusters);
    poi_pub_.publish(ped_poi);
    std_msgs::Int64 ped_no;
    ped_no.data = bounded_filter;
    ped_candNo_pub_.publish(ped_no);
    /* Verified that the following code would produce the exact same pointcloud sensor msg
    sensor_msgs::PointCloud total_clusters;
    for(int j=0;j<cloud_filtered->points.size();j++)
    {
        geometry_msgs::Point32 p;
                    p.x = cloud_filtered->points[j].x;
                    p.y = cloud_filtered->points[j].y;
                    p.z = cloud_filtered->points[j].z;
        total_clusters.points.push_back(p);
    }
    total_clusters.header = pc->header;
    cloud_pub_.publish(total_clusters);*/

}

void ped_clustering::extractCluster(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered, std::vector<pcl::PointIndices>& cluster_indices)
{
    if(cloud_filtered->size()==0) return;
    // Creating the KdTree object for the search method of the extraction
    pcl::KdTreeFLANN<pcl::PointXYZ>::Ptr tree (new pcl::KdTreeFLANN<pcl::PointXYZ>);
    tree->setInputCloud (cloud_filtered);


    pcl::EuclideanClusterExtraction<pcl::PointXYZ> ec;
    ec.setClusterTolerance (0.5);
    ec.setMinClusterSize (3);
    ec.setMaxClusterSize (100);
    ec.setSearchMethod (tree);
    ec.setInputCloud( cloud_filtered);

    ec.extract (cluster_indices);
}
void ped_clustering::filterLines(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered)
{
    // Create the segmentation object for the planar model and set all the parameters
    pcl::SACSegmentation<pcl::PointXYZ> seg;
    pcl::PointIndices::Ptr inliers (new pcl::PointIndices);
    pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_plane (new pcl::PointCloud<pcl::PointXYZ> ());

    seg.setOptimizeCoefficients (true);
    seg.setModelType (pcl::SACMODEL_LINE);
    seg.setMethodType (pcl::SAC_RANSAC);
    seg.setMaxIterations (100);
    seg.setDistanceThreshold (0.05);

    int i=0, nr_points = (int) cloud_filtered->points.size ();
    while (cloud_filtered->points.size () > 0.3 * nr_points)
    {
        // Segment the largest planar component from the remaining cloud
        seg.setInputCloud(cloud_filtered);
        seg.segment (*inliers, *coefficients); //*
        if (inliers->indices.size () == 0)
        {
            std::cout << "Could not estimate a planar model for the given dataset." << std::endl;
            break;
        }

        // Extract the planar inliers from the input cloud
        pcl::ExtractIndices<pcl::PointXYZ> extract;
        extract.setInputCloud (cloud_filtered);
        extract.setIndices (inliers);
        extract.setNegative (false);

        // Write the planar inliers to disk
        extract.filter (*cloud_plane); //*
        //std::cout << "PointCloud representing the planar component: " << cloud_plane->points.size () << " data points." << std::endl;

        // Remove the planar inliers, extract the rest
        extract.setNegative (true);
        extract.filter (*cloud_filtered); //*
    }
}
int
main (int argc, char** argv)
{
    ros::init(argc, argv, "ped_clustering");
    ped_clustering ped_cluster;
    return (0);
}
