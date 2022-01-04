#pragma once
#include <opencv2/opencv.hpp>

class Utils
{
public :
	static cv::Mat fourierTransform(const cv::Mat& originImage); // ���ظ���Ҷ�任ͼ��
	static cv::Mat getMagnitudeImage(const cv::Mat& fourierImage);  //�õ���ֵͼ��
	static cv::Mat changeCenter(const cv::Mat& magImage);
	static cv::Mat getPhaseImage(const cv::Mat& fourierImage);
	static cv::Mat inverseFourierTransform(const cv::Mat& fourierImage);
};

