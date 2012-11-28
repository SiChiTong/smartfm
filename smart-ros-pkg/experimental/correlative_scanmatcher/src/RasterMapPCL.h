/*
 * RasterMapPCL.h
 *
 *  Created on: Nov 28, 2012
 *      Author: demian
 */

#include "RasterMapImageOpt.h"
#include <sensor_msgs/PointCloud.h>

class RasterMapPCL
{
public:
	RasterMapPCL(){};

	void setInputPts(sensor_msgs::PointCloud &pc)
	{
		raster_pts_.clear();
		for(size_t i=0; i<pc.points.size(); i++)
		{
			cv::Point2f pt;
			pt.x = pc.points[i].x;
			pt.y = pc.points[i].y;
			raster_pts_.push_back(pt);
		}
	}
	transform_info getBestTf(sensor_msgs::PointCloud &pc)
	{
		vector<cv::Point2f>  query_pts;
		for(size_t i=0; i<pc.points.size(); i++)
		{
			cv::Point2f pt;
			pt.x = pc.points[i].x;
			pt.y = pc.points[i].y;
			query_pts.push_back(pt);
		}
		return findBestTf(query_pts);
	}

private:
	vector<cv::Point2f> raster_pts_;
	transform_info findBestTf(vector<cv::Point2f>& query_pts)
	{
		fmutil::Stopwatch sw;
		sw.start("New matching apporach");

		//has to be very careful with quantization error. Rule of thumb, keep ratio between step size and resolution as close to 1 as possible

		fmutil::Stopwatch sw_sub;
		sw_sub.start("1");
		cout<<query_pts.size()<<endl;
		transform_info best_info;
		RasterMapImage rm(0.2, 0.5);
		rm.getInputPoints(raster_pts_);
		best_info = rm.searchRotation(query_pts, 14.0, 2.0, M_PI, M_PI/8., best_info, true);
		cout<<"Best translation "<<best_info.translation_2d<<" "<<best_info.rotation<<" with score "<<best_info.score<<endl;
		sw_sub.end();
		sw_sub.start("2");
		RasterMapImage rm2(0.1, 0.1);
		rm2.getInputPoints(raster_pts_);
		best_info = rm2.searchRotation(query_pts, 1.5, 0.2, M_PI/16., M_PI/90., best_info, true);
		cout<<"Best translation "<<best_info.translation_2d<<" "<<best_info.rotation<<" with score "<<best_info.score<<endl;
		sw_sub.end();
		sw_sub.start("3");
		RasterMapImage rm3(0.02, 0.01);
		rm3.getInputPoints(raster_pts_);
		best_info = rm3.searchRotation(query_pts, 0.14, 0.02, M_PI/90., M_PI/360., best_info, true);
		sw_sub.end();
		cout<<"Best translation "<<best_info.translation_2d<<" "<<best_info.rotation<<" with score "<<best_info.score<<endl;

		best_info.real_pts.resize(best_info.pts.size());
		for(size_t i=0; i<best_info.pts.size();i++)
		{
			best_info.real_pts[i] = rm3.realCoordinate(best_info.pts[i]);
		}
		sw.end();
		return best_info;
	}
};
