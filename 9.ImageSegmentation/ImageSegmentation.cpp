#include "ImageSegmentation.h"

ImageSegmentation::ImageSegmentation(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	fileName = "";
	isGrowing = false;
	scene1 = new QGraphicsScene;
	scene2 = new QGraphicsScene;

	ui.graphicsView->setScene(scene1);
	ui.graphicsView_2->setScene(scene2);
	ui.label_warning->setVisible(false);
	connect(ui.btn_chooseImg, &QPushButton::clicked, this, &ImageSegmentation::btn_chooseImg);
	connect(ui.btn_hand, &QPushButton::clicked, this, &ImageSegmentation::btn_ManiSplitImg);
	connect(ui.btn_otus, &QPushButton::clicked, this, &ImageSegmentation::btn_OtusSplitImg);
	connect(ui.btn_regionGrow, &QPushButton::clicked, this, &ImageSegmentation::btn_RegionGrow);
	connect(ui.btn_regionSplit, &QPushButton::clicked, this, &ImageSegmentation::btn_RegionSplit);
	connect(ui.graphicsView, &QMyGraphicsView::updateItemPos, this, &ImageSegmentation::updateItemPos);
}

QImage ImageSegmentation::cvMat2QImage(const cv::Mat& mat)
{
	QImage img;
	img = QImage((const unsigned char*)(mat.data),
		mat.cols, mat.rows,
		mat.cols * mat.channels(),
		QImage::Format_Grayscale8);

	return img;
}
Mat ImageSegmentation::QImage2cvMat(QImage image)
{
	Mat mat;
	qDebug() << image.format();
	switch (image.format())
	{
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		mat = Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
		break;
	case QImage::Format_RGB888:
		mat = Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
		cvtColor(mat, mat, COLOR_BGR2RGB);
		break;
	case QImage::Format_Indexed8:
		mat = Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
		break;
	}
	return mat;
}
void ImageSegmentation::btn_chooseImg()
{
	//��ͼƬ
	fileName = QFileDialog::getOpenFileName(this,
		tr("Open Input Image"),
		QDir::currentPath(),
		tr("image") + "(*.jpg *.png *bmp)");
	if (fileName != "")
	{
		image1 = imread(fileName.toStdString(), IMREAD_GRAYSCALE);
		qDebug() << image1.rows << image1.cols;
		if (!image1.empty())
		{
			isGrowing = false;
			ui.label_warning->setVisible(false);
			scene1->clear();
			qimage1 = cvMat2QImage(image1);
			qDebug() << qimage1.width() << qimage1.height();
			scene1->addPixmap(QPixmap::fromImage(qimage1));
			tmpGrowingMask = Mat::zeros(image1.size(), CV_8UC1);
		}
	}
}

void ImageSegmentation::btn_ManiSplitImg()
{
	isGrowing = false;
	ui.label_warning->setVisible(false);
	tmpGrowingMask = Mat::zeros(image1.size(), CV_8UC1);
	scene2->clear();
	int v = ui.spin_threshold->value();
	threshold(image1, image2, v, 255, THRESH_BINARY);
	qimage2 = cvMat2QImage(image2);
	scene2->addPixmap(QPixmap::fromImage(qimage2));
}

void ImageSegmentation::btn_OtusSplitImg()
{
	isGrowing = false;
	ui.label_warning->setVisible(false);
	tmpGrowingMask = Mat::zeros(image1.size(), CV_8UC1);
	scene2->clear();
	threshold(image1, image2, 0, 255, THRESH_OTSU);
	qimage2 = cvMat2QImage(image2);
	scene2->addPixmap(QPixmap::fromImage(qimage2));
}

void ImageSegmentation::btn_RegionGrow()
{
	isGrowing = true;
	ui.label_warning->setVisible(true);
	tmpGrowingMask = Mat::zeros(image1.size(), CV_8UC1);
	qimage2 = cvMat2QImage(tmpGrowingMask);
	scene2->addPixmap(QPixmap::fromImage(qimage2));
}

void ImageSegmentation::btn_RegionSplit()
{
	isGrowing = false;
	ui.label_warning->setVisible(false);
	tmpGrowingMask = Mat::zeros(image1.size(), CV_8UC1);
	scene2->clear();
	RegionCut(image1, image2);
	qimage2 = cvMat2QImage(image2);
	scene2->addPixmap(QPixmap::fromImage(qimage2));
}

void ImageSegmentation::updateItemPos(QPointF pos)
{
	//���ݻ�õ�����,����RGB����ֵ
	int x = pos.x();
	int y = pos.y();
	int th = ui.spin_growthreshold->value();
	QString str = "(" + QString::number(x) + "," + QString::number(y) + ")";
	ui.label_coordinate->setText(str);
	scene2->clear();
	image2 = RegionGrow(image1, Point2i(x, y), th);
	bitwise_or(image2, tmpGrowingMask, image2);    //��ǰһ���ָ��ͼ��������
	tmpGrowingMask = image2.clone();    //����ǰһ���ָ�ͼ
	//image2 = tmpGrowingMask + image2;
	for (int i = 0;i < image2.rows;i++) {
		for (int j = 0;j < image2.cols;j++) {
			if (image2.at<uchar>(i, j) != 0)
				image2.at<uchar>(i, j) = 255;
		}
	}
	qimage2 = cvMat2QImage(image2);
	scene2->addPixmap(QPixmap::fromImage(qimage2));
}
void ImageSegmentation::RegionCut(Mat src, Mat& src1) {
	struct SplitStruct {
		int width;  //ͼ��Ŀ��
		int height;  //ͼ��ĸ߶�
		int x;    //���ԭͼ�����ݵ�ƫ�ƿ��
		int y;    //���ԭͼ�����ݵ�ƫ�Ƹ߶�
	};
	stack<SplitStruct> splitPt;
	SplitStruct ss, ssTemp;
	ss.width = src.rows;
	ss.height = src.cols;
	ss.x = 0;
	ss.y = 0;
	splitPt.push(ss);
	src.copyTo(src1);
	int values[2][2];//�洢ÿ�η���������
	double sn[2][2];
	int Height[3], Width[3];//�洢ÿ�η��Ѻ�û����Ĵ�С

	while (!splitPt.empty()) {
		ss = splitPt.top();//��ջ��Ԫ�ط�ֵ���ṹ��
		splitPt.pop();   //ջ��Ԫ�س�ջ

		Height[0] = 0;
		Height[1] = (ss.height + 1) / 2;//��һ����ĸ�
		Height[2] = ss.height - Height[1];//�ڶ�����ĸ�

		Width[0] = 0;
		Width[1] = (ss.width + 1) / 2;  //��һ����Ŀ�
		Width[2] = ss.width - Width[1];//�ڶ�����Ŀ�

		for (int i = 1;i < 3;i++) {
			for (int j = 1;j < 3;j++) {
				//����ÿһ�������ƽ���Ҷ�ֵ
				int Sumgray = 0;
				for (int p = 0;p < Width[i];p++) {
					for (int q = 0;q < Height[j];q++) {
						Sumgray += src.at<uchar>(p + ss.x, q + ss.y);
					}
				}
				if (Width[i] * Height[j] == 0)
					continue;
				if (Width[i] * Height[j] == 1) {    //���ֵ���С����
					if (src.at<uchar>(Width[i - 1] + ss.x, Height[j - 1] + ss.y) < 100) {
						src1.at<uchar>(Width[i - 1] + ss.x, Height[j - 1] + ss.y) = 0;
					}
					else {
						src1.at<uchar>(Width[i - 1] + ss.x, Height[j - 1] + ss.y) = 255;
					}
					continue;
				}
				values[i - 1][j - 1] = Sumgray / (Height[j] * Width[i]);

				//�ж��Ƿ���Ҫ��������
				double sngray = 0.0;
				for (int p = 0;p < Width[i];p++) {
					for (int q = 0;q < Height[j];q++) {
						//���㷽��
						sngray += (src.at<uchar>(p + Width[i - 1] + ss.x, q + Height[j - 1] + ss.y) - values[i - 1][j - 1]) * (src.at<uchar>(p + Width[i - 1] + ss.x, q + Height[j - 1] + ss.y) - values[i - 1][j - 1]);
					}
				}
				sn[i - 1][j - 1] = sngray / (Height[j] * Width[i]);

				//�ж��Ƿ���Ҫ��������
				if (sn[i - 1][j - 1] > 16) {   //����������ڵĻҶ�ֵ�������趨��ֵ����������ѣ�����ϲ�
					//�����µ�����Ĵ�С�Լ����ԭ�����ص�ƫ�Ƶ�λ��
					ssTemp.width = Width[i];
					ssTemp.height = Height[j];
					ssTemp.x = ss.x + Width[i - 1];
					ssTemp.y = ss.y + Height[j - 1];
					splitPt.push(ssTemp);          //���µĽṹ��Ԫ����ջ
				}
				else {     //���������������ϲ�
					for (int xx = 0;xx < Width[i];xx++) {
						for (int yy = 0;yy < Height[j];yy++) {
							src1.at<uchar>(xx + Width[i - 1] + ss.x, yy + Height[j - 1] + ss.y) = 0;   //���Ѹ������ڵ�����������src1����0�ĻҶȻ��Ƴ���
						}
					}
				}
			}
		}
	}
}

Mat ImageSegmentation::RegionGrow(Mat src, Point2i pt, int th)
{
	Point2i ptGrowing;						//��������λ��
	int nGrowLable = 0;								//����Ƿ�������
	int nSrcValue = 0;								//�������Ҷ�ֵ
	int nCurValue = 0;								//��ǰ������Ҷ�ֵ
	Mat matDst = Mat::zeros(src.size(), CV_8UC1);	//����һ���հ��������Ϊ��ɫ
	//��������˳������
	int DIR[8][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 } };
	vector<Point2i> vcGrowPt;						//������ջ
	vcGrowPt.push_back(pt);							//��������ѹ��ջ��
	matDst.at<uchar>(pt.y, pt.x) = 255;				//���������
	nSrcValue = src.at<uchar>(pt.y, pt.x);			//��¼������ĻҶ�ֵ

	while (!vcGrowPt.empty())						//����ջ��Ϊ��������
	{
		pt = vcGrowPt.back();						//ȡ��һ��������
		vcGrowPt.pop_back();

		//�ֱ�԰˸������ϵĵ��������
		for (int i = 0; i < 9; ++i)
		{
			ptGrowing.x = pt.x + DIR[i][0];
			ptGrowing.y = pt.y + DIR[i][1];
			//����Ƿ��Ǳ�Ե��
			if (ptGrowing.x < 0 || ptGrowing.y < 0 || ptGrowing.x >(src.cols - 1) || (ptGrowing.y > src.rows - 1))
				continue;

			nGrowLable = matDst.at<uchar>(ptGrowing.y, ptGrowing.x);		//��ǰ��������ĻҶ�ֵ

			if (nGrowLable == 0)					//�����ǵ㻹û�б�����
			{
				nCurValue = src.at<uchar>(ptGrowing.y, ptGrowing.x);
				if (abs(nSrcValue - nCurValue) < th)					//����ֵ��Χ��������
				{
					matDst.at<uchar>(ptGrowing.y, ptGrowing.x) = 255;		//���Ϊ��ɫ
					vcGrowPt.push_back(ptGrowing);					//����һ��������ѹ��ջ��
				}
			}
		}
	}
	return matDst.clone();
}