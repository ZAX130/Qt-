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

cv::Mat Utils::LogfourierTransform(const cv::Mat& originImage) {
	cv::Mat paddingImage;

	//�õ��ʺϸ���Ҷ�任�����ųߴ�
	int m = cv::getOptimalDFTSize(originImage.rows);
	int n = cv::getOptimalDFTSize(originImage.cols);

	//��䣬�ϲ����಻���
	cv::copyMakeBorder(originImage, paddingImage, 0, m - originImage.rows,
		0, n - originImage.cols, cv::BORDER_CONSTANT, cv::Scalar(0));
	paddingImage.convertTo(paddingImage, CV_32FC1);
	cv::log(paddingImage + 1, paddingImage);
	//paddingImage = paddingImage * 255 / log(255);
	//idft�и�bug,���������ǳ���,�����һ���ָܻ�ԭͼ��,����������ǿ����ԭͼ��0~255,���Ǽ�¼��ԭͼ��������Сֵ,��ѹ����ȥ
	//˫ͨ��Mat
	cv::Mat planes[] = { cv::Mat_<float>(paddingImage), cv::Mat::zeros(paddingImage.size(), CV_32FC1) };
	cv::Mat mergeImage;
	cv::merge(planes, 2, mergeImage);
	//cout << "-------------------merge--------------------------------------" << format(mergeImage(cv::Rect(50, 50, 3, 3)), cv::Formatter::FMT_PYTHON) << endl;
	//���и���Ҷ�任
	cv::dft(mergeImage, mergeImage, cv::DFT_COMPLEX_OUTPUT);
	//cout << "-------------------merge--------------------------------------" << format(mergeImage(cv::Rect(50,50,3,3)), cv::Formatter::FMT_PYTHON) << endl;
	return mergeImage;
}
cv::Mat Utils::expinverseFourierTransform(const cv::Mat& fourierImage) {
	cv::Mat dealtImage = fourierImage.clone();
	//cout << "-------------------dealt--------------------------------------" << format(dealtImage(cv::Rect(50, 50, 3, 3)), cv::Formatter::FMT_PYTHON) << endl;
	cv::Mat iDFT[] = { cv::Mat::zeros(dealtImage.size(), CV_32FC1),
					  cv::Mat::zeros(dealtImage.size(), CV_32FC1) };

	cv::idft(dealtImage, dealtImage);
	//cout << "-------------------idft dealt--------------------------------------" << format(dealtImage(cv::Rect(50, 50, 3, 3)), cv::Formatter::FMT_PYTHON) << endl;
	cv::split(dealtImage, iDFT);
	//cv::exp(iDFT[0], iDFT[0]);

	normalize(iDFT[0], iDFT[0], 1, log(256), cv::NORM_MINMAX);
	cv::exp(iDFT[0], iDFT[0]);
	iDFT[0] -= 1;
	//cout << "-------------------idft dealt--------------------------------------" << format(iDFT[0](cv::Rect(50, 50, 3, 3)), cv::Formatter::FMT_PYTHON) << endl;
	iDFT[0].convertTo(iDFT[0], CV_8UC1, 1, 0);
	//cout << "-------------------idft dealt--------------------------------------" << format(iDFT[0](cv::Rect(50, 50, 3, 3)), cv::Formatter::FMT_PYTHON) << endl;
	return iDFT[0];
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

// ͼ��߽紦��
cv::Mat Utils::image_make_border(cv::Mat& src)
{
	int w = cv::getOptimalDFTSize(src.cols); // ��ȡDFT�任����ѿ��
	int h = cv::getOptimalDFTSize(src.rows); // ��ȡDFT�任����Ѹ߶�

	cv::Mat padded;
	// ����������ͼ��߽磬���� = 0
	cv::copyMakeBorder(src, padded, 0, h - src.rows, 0, w - src.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	padded.convertTo(padded, CV_32FC1);
	return padded;
}

cv::Mat Utils::changeCenter(const cv::Mat& magImage) {
	cv::Mat magImageCopy = magImage.clone();
	//����������л��У���ת��Ϊż��
	magImageCopy = magImage(cv::Rect(0, 0, magImage.cols - (magImage.cols % 2), magImage.rows - (magImage.rows % 2)));
	//�����Ų�����Ҷ�任���ͼ��ʹ��ԭ��λ��ͼ������
	int centerX = magImageCopy.cols / 2;
	int centerY = magImageCopy.rows / 2;

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

// ������˹��ͨ�˲��˺���
cv::Mat Utils::butterworth_low_kernel(cv::Mat& scr, float sigma, int n)
{
	cv::Mat butterworth_low_pass(scr.size(), CV_32FC1); //��CV_32FC1
	float D0 = sigma;//�뾶D0ԽС��ģ��Խ�󣻰뾶D0Խ��ģ��ԽС
	for (int i = 0; i < scr.rows; i++) {
		for (int j = 0; j < scr.cols; j++) {
			float d = sqrt(pow(float(i - scr.rows / 2), 2) + pow(float(j - scr.cols / 2), 2));//����,����pow����Ϊfloat��
			butterworth_low_pass.at<float>(i, j) = 1.0f / (1.0f + pow(d / D0, 2 * n));
		}
	}
	return butterworth_low_pass;
}

// ������˹��ͨ�˲�
cv::Mat Utils::getButterworth_low_kernel(cv::Mat& src, float d0, int n)
{
	// H = 1 / (1+(D/D0)^2n)   n��ʾ������˹�˲����Ĵ���
	// ����n=1 ������͸�ֵ    ����n=2 ��΢����͸�ֵ  ����n=5 ��������͸�ֵ   ����n=20 ��ILPF����
	cv::Mat padded = image_make_border(src);
	cout << "����pad��" << padded.size().height << padded.size().width;
	cv::Mat butterworth_kernel = butterworth_low_kernel(padded, d0, n);
	return butterworth_kernel;
}

// ������˹��ͨ�˲��˺���
cv::Mat Utils::butterworth_high_kernel(cv::Mat& scr, float sigma, int n)
{
	cv::Mat butterworth_high_pass(scr.size(), CV_32FC1); //��CV_32FC1
	float D0 = (float)sigma;  // �뾶D0ԽС��ģ��Խ�󣻰뾶D0Խ��ģ��ԽС
	for (int i = 0; i < scr.rows; i++) {
		for (int j = 0; j < scr.cols; j++) {
			float d = sqrt(pow(float(i - scr.rows / 2), 2) + pow(float(j - scr.cols / 2), 2));//����,����pow����Ϊfloat��
			butterworth_high_pass.at<float>(i, j) = 1.0f - 1.0f / (1.0f + pow(d / D0, 2 * n));
		}
	}
	return butterworth_high_pass;
}

// ������˹��ͨ�˲�
cv::Mat Utils::getButterworth_high_kernel(cv::Mat& src, float d0, int n)
{
	// H = 1 / (1+(D/D0)^2n)   n��ʾ������˹�˲����Ĵ���
	// ����n=1 ������͸�ֵ    ����n=2 ��΢����͸�ֵ  ����n=5 ��������͸�ֵ   ����n=20 ��ILPF����
	cv::Mat padded = image_make_border(src);
	cout << "����pad��" << padded.size().height << padded.size().width;
	cv::Mat butterworth_kernel = butterworth_high_kernel(padded, d0, n);
	return butterworth_kernel;
}

// �����ͨ�˲��˺���
cv::Mat Utils::ideal_low_kernel(cv::Mat& scr, float sigma)
{
	cv::Mat ideal_low_pass(scr.size(), CV_32FC1); //��CV_32FC1
	float D0 = sigma;//�뾶D0ԽС��ģ��Խ�󣻰뾶D0Խ��ģ��ԽС
	for (int i = 0; i < scr.rows; i++) {
		for (int j = 0; j < scr.cols; j++) {
			float d_square = pow(float(i - scr.rows / 2), 2) + pow(float(j - scr.cols / 2), 2);//����,����pow����Ϊfloat��
			ideal_low_pass.at<float>(i, j) = d_square <= D0 * D0 ? 1.0f : 0.0f;
		}
	}
	return ideal_low_pass;
}

// �����ͨ�˲�
cv::Mat Utils::getideal_low_kernel(cv::Mat& src, float d0)
{
	cv::Mat padded = image_make_border(src);
	cout << "����pad��" << padded.size().height << padded.size().width;
	cv::Mat ideal_kernel = ideal_low_kernel(padded, d0);
	return ideal_kernel;
}

// �����ͨ�˲��˺���
cv::Mat Utils::ideal_high_kernel(cv::Mat& scr, float sigma)
{
	cv::Mat ideal_high_pass(scr.size(), CV_32FC1); //��CV_32FC1
	float D0 = sigma;//�뾶D0ԽС��ģ��Խ�󣻰뾶D0Խ��ģ��ԽС
	for (int i = 0; i < scr.rows; i++) {
		for (int j = 0; j < scr.cols; j++) {
			float d_square = pow(float(i - scr.rows / 2), 2) + pow(float(j - scr.cols / 2), 2);
			ideal_high_pass.at<float>(i, j) = d_square > D0 * D0 ? 1.0f : 0.0f;
		}
	}
	return ideal_high_pass;
}

// �����ͨ�˲�
cv::Mat Utils::getideal_high_kernel(cv::Mat& src, float d0)
{
	cv::Mat padded = image_make_border(src);
	cout << "����pad��" << padded.size().height << padded.size().width;
	cv::Mat ideal_kernel = ideal_high_kernel(padded, d0);
	return ideal_kernel;
}

// ̬ͬ�˲��˺���
cv::Mat Utils::homo_kernel(cv::Mat& scr, float gammaH, float gammaL, float sigma, int n)
{
	cv::Mat homo(scr.size(), CV_32FC1); //��CV_32FC1
	float D0 = (float)sigma;  // �뾶D0
	for (int i = 0; i < scr.rows; i++) {
		for (int j = 0; j < scr.cols; j++) {
			float d_square = pow(float(i - scr.rows / 2), 2) + pow(float(j - scr.cols / 2), 2);
			homo.at<float>(i, j) = (gammaH - gammaL) * (1.0f - exp(-n * d_square / (D0 * D0))) + gammaL;
		}
	}
	return homo;
}

// ̬ͬ�˲�
cv::Mat Utils::gethomo_kernel(cv::Mat& src, float gammaH, float gammaL, float d0, int n)
{
	cv::Mat padded = image_make_border(src);
	cout << "����pad��" << padded.size().height << padded.size().width;
	cv::Mat homo = homo_kernel(padded, gammaH, gammaL, d0, n);
	return homo;
}

cv::Mat Utils::filtered_fourier(cv::Mat& blur, const cv::Mat& fourierImage)
{
	cv::Mat dealtImage = fourierImage.clone();
	cv::Mat plane[] = { cv::Mat::zeros(dealtImage.size(), CV_32FC1),
					  cv::Mat::zeros(dealtImage.size(), CV_32FC1) };
	cv::split(fourierImage, plane);

	// ���µĲ�����Ƶ��Ǩ��
	plane[0] = changeCenter(plane[0]);
	plane[1] = changeCenter(plane[1]);
	// *****************�˲���������DFT����ĳ˻�****************
	blur = blur(cv::Rect(0, 0, blur.cols - (blur.cols % 2), blur.rows - (blur.rows % 2)));
	cv::Mat blur_r, blur_i, BLUR;
	cv::multiply(plane[0], blur, blur_r);  // �˲���ʵ�����˲���ģ���ӦԪ����ˣ�
	cv::multiply(plane[1], blur, blur_i);  // �˲����鲿���˲���ģ���ӦԪ����ˣ�
	cv::Mat plane1[] = { blur_r, blur_i };

	// �ٴΰ��ƻ���������任
	plane1[0] = changeCenter(plane1[0]);
	plane1[1] = changeCenter(plane1[1]);
	cv::merge(plane1, 2, BLUR); // ʵ�����鲿�ϲ�
	return BLUR;
}

cv::Mat Utils::ToShowFormat(cv::Mat& image)
{
	cv::Mat result = image.clone();
	normalize(result, result, 0, 1, cv::NORM_MINMAX);
	result.convertTo(result, CV_8UC1, 255, 0);
	return result;
}