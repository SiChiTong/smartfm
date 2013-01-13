/*
 * renderMap.h
 *
 *  Created on: Dec 6, 2012
 *      Author: demian
 */


class RenderMap
{
public:
	RenderMap(): min_pt_(1e99,1e99), max_pt_(-1e99,-1e99)
	{}

	cv::Point2f min_pt_, max_pt_;
	cv::Mat image_;
	cv::Mat skel_;
	double res_;
	template <class T>
	inline cv::Point imageCoordinate(T pt, double res_)
	{
		return cv::Point(round((pt.x-min_pt_.x)/res_), round(image_.rows -(pt.y-min_pt_.y)/res_));
	}

	vector<cv::Point2f> mapToRealPts()
	{

		vector<cv::Point2f> real_pts;
		for(int i=0; i<skel_.cols; i++)
		{
			for(int j=0; j<skel_.rows; j++)
			{
				if(skel_.data[j*skel_.cols + i] == 255)
					real_pts.push_back(cv::Point2f(i * res_ + min_pt_.x, -(j- image_.rows)*res_ + min_pt_.y ));
			}
		}
		return real_pts;
	}

	void drawDensityMap(vector<geometry_msgs::Point32> &raster_pt, double res)
	{
		fmutil::Stopwatch sw;
		res_ = res;
		sw.start("Raster Map");

		//very fast process, only needs 5 ms on 1620 points with 1000 loops

		for(size_t i=0; i<raster_pt.size(); i++)
		{
			if(raster_pt[i].x < min_pt_.x) min_pt_.x = raster_pt[i].x;
			if(raster_pt[i].x > max_pt_.x) max_pt_.x = raster_pt[i].x;

			if(raster_pt[i].y < min_pt_.y) min_pt_.y = raster_pt[i].y;
			if(raster_pt[i].y > max_pt_.y) max_pt_.y = raster_pt[i].y;
		}

		//cout << "MinMax "<<min_pt_ << " " <<max_pt_<<endl;
		cv::Point2f map_size(max_pt_.x - min_pt_.x, max_pt_.y - min_pt_.y);
		map_size.x = ceil(map_size.x/res_); map_size.y =ceil(map_size.y/res_);
		//cout << "Size "<<map_size<<endl;
		//lost about 10ms when using 32F instead of 8U, and total of 45 ms if draw circle function is called, perhaps too much
		cv::Mat dense_map = cv::Mat::zeros( (int) map_size.y, (int) map_size.x  , CV_32FC1);
		image_ = cv::Mat::zeros( (int) map_size.y, (int) map_size.x  , CV_8UC1);
		for(size_t i=0; i<raster_pt.size(); i++)
		{
			cv::Point pt =imageCoordinate(raster_pt[i], res_);
			if(pt.x >= dense_map.cols || pt.y >= dense_map.rows) continue;
			double pt_height = raster_pt[i].z;
			int height = round(fmutil::bound(0.0,pt_height, 2.0)/2.0*254.0)+1;
			if(dense_map.at<float>(pt.y, pt.x)<1.0)
				dense_map.at<float>(pt.y, pt.x) += 0.01;

		}

		for(int x=0; x<image_.cols; x++)
		{
			for(int y=0; y<image_.rows; y++)
			{
				image_.at<uchar>(y,x) = (int)(dense_map.at<float>(y,x) *255.);
			}
		}
	}
	void drawHeightMap(vector<geometry_msgs::Point32> raster_pt, double res, double size_x, double size_y)
	{
		fmutil::Stopwatch sw;
		res_ = res;
		sw.start("Raster Map");

		//very fast process, only needs 5 ms on 1620 points with 1000 loops
		double pad_x = size_x/2.;
		double pad_y = size_y/2.;
		for(size_t i=0; i<raster_pt.size(); i++)
		{
			raster_pt[i].x += pad_x;
			raster_pt[i].y += pad_y;
			/*if(raster_pt[i].x < min_pt_.x) min_pt_.x = raster_pt[i].x;
			if(raster_pt[i].x > max_pt_.x) max_pt_.x = raster_pt[i].x;

			if(raster_pt[i].y < min_pt_.y) min_pt_.y = raster_pt[i].y;
			if(raster_pt[i].y > max_pt_.y) max_pt_.y = raster_pt[i].y;*/
		}
		max_pt_.x = size_x;
		max_pt_.y = size_y;
		min_pt_.x = min_pt_.y = 0.;

		//cout << "MinMax "<<min_pt_ << " " <<max_pt_<<endl;
		cv::Point2f map_size(size_x, size_y);//max_pt_.x - min_pt_.x, max_pt_.y - min_pt_.y);
		map_size.x = ceil(map_size.x/res_); map_size.y =ceil(map_size.y/res_);
		//cout << "Size "<<map_size<<endl;
		//lost about 10ms when using 32F instead of 8U, and total of 45 ms if draw circle function is called, perhaps too much
		image_ = cv::Mat::zeros( (int) map_size.y, (int) map_size.x  , CV_8UC1);

		for(size_t i=0; i<raster_pt.size(); i++)
		{
			cv::Point pt =imageCoordinate(raster_pt[i], res_);
			if(pt.x >= image_.cols || pt.y >= image_.rows || pt.x < 0 || pt.y < 0) continue;
			double pt_height = raster_pt[i].z;
			int height = round(fmutil::bound(0.0,pt_height, 2.0)/2.0*254.0)+1;
			if(image_.data[pt.y*image_.cols + pt.x] < height)
				image_.data[pt.y*image_.cols + pt.x] = height;

		}
	}
	template <class T>
	void drawMap(vector<T> &raster_pt, double res, bool morph = true)
	{
		fmutil::Stopwatch sw;
		res_ = res;
		sw.start("Raster Map");

		//very fast process, only needs 5 ms on 1620 points with 1000 loops

		for(size_t i=0; i<raster_pt.size(); i++)
		{
			if(raster_pt[i].x < min_pt_.x) min_pt_.x = raster_pt[i].x;
			if(raster_pt[i].x > max_pt_.x) max_pt_.x = raster_pt[i].x;

			if(raster_pt[i].y < min_pt_.y) min_pt_.y = raster_pt[i].y;
			if(raster_pt[i].y > max_pt_.y) max_pt_.y = raster_pt[i].y;
		}

		//cout << "MinMax "<<min_pt_ << " " <<max_pt_<<endl;
		cv::Point2f map_size(max_pt_.x - min_pt_.x, max_pt_.y - min_pt_.y);
		map_size.x = ceil(map_size.x/res_); map_size.y =ceil(map_size.y/res_);
		//cout << "Size "<<map_size<<endl;
		//lost about 10ms when using 32F instead of 8U, and total of 45 ms if draw circle function is called, perhaps too much
		image_ = cv::Mat::zeros( (int) map_size.y, (int) map_size.x  , CV_8UC1);

		for(size_t i=0; i<raster_pt.size(); i++)
		{
			cv::Point pt =imageCoordinate(raster_pt[i], res_);
			if(pt.x >= image_.cols || pt.y >= image_.rows) continue;

			image_.data[pt.y*image_.cols + pt.x] = 255;

		}
		//if(cv::imwrite(string("test_render.png"), image_))
					//cout<<"Write image success"<<endl;
		cv::Mat img = image_.clone();
		if(morph)
		{
			cv::Mat skel(img.size(), CV_8UC1, cv::Scalar(0));
			cv::Mat temp;
			cv::Mat eroded;

			cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

			bool done;
			cv::dilate(img, img, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(5, 5)));
			cv::dilate(img, img, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3)));
			//cv::imwrite(string("test_render2.png"), img);
			do
			{
				cv::erode(img, eroded, element);
				cv::dilate(eroded, temp, element); // temp = open(img)
				cv::subtract(img, temp, temp);
				cv::bitwise_or(skel, temp, skel);
				eroded.copyTo(img);

				done = (cv::norm(img) == 0);
			} while (!done);
			//image_  = img;

			//cout<<"Writing image"<<endl;


			skel_ = skel;
		}
		else skel_ = img.clone();
		//cv::imwrite(string("test_render3.png"), image_);
		sw.end(false);
	}
	void drawMap(vector<geometry_msgs::Point32> raster_pt, double res_, string filename)
	{
		drawMap(raster_pt, res_);
		cv::imwrite(filename, image_);
	}
};
