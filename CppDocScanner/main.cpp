#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat imgOrig;
Mat imgGray; 
Mat imgBlur; 
Mat imgCanny; 
Mat imgDil;
Mat imgWarp;

Mat preProcessinrgImg(Mat img2proc)
{
	Mat imgBlur2; 
	Mat imgCanny2;
	Mat imgBlur3;
	Mat imgCanny3;

	cvtColor(img2proc, imgGray, COLOR_BGR2GRAY);
	bilateralFilter(imgGray, imgBlur, 10, 100, 100);
	Canny(imgBlur, imgCanny, 90, 180);
	GaussianBlur(imgCanny, imgBlur2, Size(1, 1), 2, 0);
	GaussianBlur(imgBlur2, imgBlur3, Size(1, 1), 2, 0);
	Canny(imgBlur3, imgCanny2, 40, 100);
	//Canny(imgCanny2, imgCanny3, 10, 60);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny2, imgDil, kernel);

	return imgDil;
}

vector<Point> getContoursImg(Mat image2fc)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image2fc, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	vector<vector<Point>> conPolygon(contours.size());
	vector<Rect> boundRectangle(contours.size());
	vector<Point> bOne;
	int maxArea = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		string objectType;

		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPolygon[i], 0.03 * peri, true);

			if (area > maxArea && conPolygon[i].size() == 4)
			{
				bOne = { conPolygon[i][0], conPolygon[i][1], conPolygon[i][2], conPolygon[i][3] };
				maxArea = area;
			}
		}
	}
	return bOne;
}

void drawPoints(vector<Point> points, Scalar color)
{
	for (int i = 0; i < points.size(); i++)
	{
		circle(imgOrig, points[i], 5, color, FILLED);
		putText(imgOrig, to_string(i), points[i], FONT_HERSHEY_PLAIN, 3, color, 5);
	}
}

vector<Point> reorderPoints(vector<Point> points)
{
	vector<Point> newPoints;
	vector<int> summarizedPoints;
	vector<int> substractedPoints;

	for (int i = 0; i < 4; i++)
	{
		summarizedPoints.push_back(points[i].x + points[i].y);
		substractedPoints.push_back(points[i].x - points[i].y);
	}

	newPoints.push_back(points[min_element(summarizedPoints.begin(), summarizedPoints.end()) - summarizedPoints.begin()]); //0
	newPoints.push_back(points[max_element(substractedPoints.begin(), substractedPoints.end()) - substractedPoints.begin()]); //1
	newPoints.push_back(points[min_element(substractedPoints.begin(), substractedPoints.end()) - substractedPoints.begin()]); //2
	newPoints.push_back(points[max_element(summarizedPoints.begin(), summarizedPoints.end()) - summarizedPoints.begin()]); //3
	
	return newPoints;
}

Mat getWarp(Mat img, vector<Point> points, float w, float h)
{
	Point2f src[4] = { points[0],points[1],points[2],points[3] };
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));

	return imgWarp;
}

void fileSave(Mat imgWarp)
{
	imwrite("ScannedDocument.jpg", imgWarp);
}

void main()
{
	float wA4 = 768;
	float hA4 = 1024;
	string path = "test1_img.jpg";
	imgOrig = imread(path);
	resize(imgOrig, imgOrig, Size(768, 1024));

	Mat imgNew = preProcessinrgImg(imgOrig);
	vector<Point> initialPoints = getContoursImg(imgNew);
	vector<Point> rePoints;
	//cout << initialPoints << endl;
	//drawPoints(initialPoints, Scalar(0, 0, 255));
	rePoints = reorderPoints(initialPoints);
	drawPoints(rePoints, Scalar(0, 0, 255));

	imgWarp = getWarp(imgOrig, rePoints, wA4, hA4);

	imshow("Test img", imgOrig);
	imshow("Dilation img", imgNew);
	//imshow("Warp img", imgWarp);

	//fileSave(imgWarp);

	waitKey(0);
}