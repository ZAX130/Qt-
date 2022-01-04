#include "Utils.h"

cv::Mat Utils::fourierTransform(const cv::Mat& originImage) {
	cv::Mat paddingImage;

	//�õ��ʺϸ���Ҷ�任�����ųߴ�
	int m = cv::getOptimalDFTSize(originImage.rows);
	int n = cv::getOptimalDFTSize(originImage.cols);

	//��䣬�ϲ����಻���
	cv::copyMakeBorder(originImage, paddingImage, 0, m - originImage.rows,
		0, n - originImage.cols, cv::BORDER_CONSTANT, cv::Scalar(0));

	//˫ͨ��Mat
	cv::Mat planes[] = { cv::Mat_<float>(paddingImage), cv::Mat::zeros(paddingImage.size(), CV_32FC1) };
	cv::Mat mergeImage;
	cv::merge(planes, 2, mergeImage);

	//���и���Ҷ�任
	cv::dft(mergeImage, mergeImage, cv::DFT_COMPLEX_OUTPUT);

	return mergeImage;
}

cv::Mat Utils::changeCenter(const cv::Mat& magImage) {
	//�����Ų�����Ҷ�任���ͼ��ʹ��ԭ��λ��ͼ������
	int centerX = magImage.cols / 2;
	int centerY = magImage.rows / 2;

	cv::Mat magImageCopy = magImage.clone();
	cv::Mat planes[] = { cv::Mat::zeros(magImageCopy.size(), CV_32FC1),
						cv::Mat::zeros(magImageCopy.size(), CV_32FC1) };

	cv::Mat mat1(magImageCopy, cv::Rect(0, 0, centerX, centerY));				//����
	cv::Mat mat2(magImageCopy, cv::Rect(0, centerY, centerX, centerY));			//����
	cv::Mat mat3(magImageCopy, cv::Rect(centerX, 0, centerX, centerY));				//����
	cv::Mat mat4(magImageCopy, cv::Rect(centerX, centerY, centerX, centerY));	//����

	//�������Ϻ�����
	cv::Mat tempImage;
	mat1.copyTo(tempImage);
	mat4.copyTo(mat1);
	tempImage.copyTo(mat4);

	//�������º�����
	mat2.copyTo(tempImage);
	mat3.copyTo(mat2);
	tempImage.copyTo(mat3);

	return magImageCopy;
}

cv::Mat Utils::getMagnitudeImage(const cv::Mat& fourierImage) {
	cv::Mat planes[] = { cv::Mat::zeros(fourierImage.size(), CV_32FC1),
						cv::Mat::zeros(fourierImage.size(), CV_32FC1) };

	cv::Mat magImage = planes[0].clone();
	cv::split(fourierImage, planes);
	cv::magnitude(planes[0], planes[1], magImage);

	//����������л��У���ת��Ϊż��
	magImage = magImage(cv::Rect(0, 0, magImage.cols - (magImage.cols % 2), magImage.rows - (magImage.rows % 2)));


	magImage = changeCenter(magImage);

	//�����ֵ,��ת���������߶�
	//ȡ������Ŀ����ʹ��Щ����ϵ͵ĳɷ���Ը�����ɷֵ������ߣ��Ա�۲��ڸ��ڵͷ������е������źš�
	cv::log(magImage + 1, magImage);

	//��һ��������0-1֮��ĸ�����������任Ϊ���ӵ�ͼ���ʽ
	cv::normalize(magImage, magImage, 0, 1, cv::NORM_MINMAX);

	//cv::imshow("test2", magImage);
	magImage.convertTo(magImage, CV_8UC1, 255, 0);
	return magImage;
}



cv::Mat Utils::getPhaseImage(const cv::Mat& fourierImage) {
	//�����λ��
	cv::Mat planes[] = { cv::Mat::zeros(fourierImage.size(), CV_32FC1),
						cv::Mat::zeros(fourierImage.size(), CV_32FC1) };

	cv::Mat phaseImage = planes[0].clone();
	cv::split(fourierImage, planes);
	cv::phase(planes[0], planes[1], phaseImage);
	phaseImage = changeCenter(phaseImage);
	//��һ��������0-1֮��ĸ�����������任Ϊ���ӵ�ͼ���ʽ
	cv::normalize(phaseImage, phaseImage, 0, 1, cv::NORM_MINMAX); //��һ�� ������ʾ����ʵ������û�й�ϵ

	phaseImage.convertTo(phaseImage, CV_8UC1, 255, 0);
	return phaseImage;
}

cv::Mat Utils::inverseFourierTransform(const cv::Mat& fourierImage) {
	cv::Mat dealtImage = fourierImage.clone();
	cv::Mat iDFT[] = { cv::Mat::zeros(dealtImage.size(), CV_32FC1),
					  cv::Mat::zeros(dealtImage.size(), CV_32FC1) };

	cv::idft(dealtImage, dealtImage);
	cv::split(dealtImage, iDFT);
	normalize(iDFT[0], iDFT[0], 0, 1, cv::NORM_MINMAX);
	iDFT[0].convertTo(iDFT[0], CV_8UC1, 255, 0);
	return iDFT[0];
}
