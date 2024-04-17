#include "Algorithm.h"
#include <QString>
#include "uStatus.h"
#include "ct_image.h"
#include <thread>
#include <vtkInformation.h>
#include <QtConcurrent/QtConcurrent>
#include <QProgressDialog>
#include "uiHelper.h"

namespace ImageProcess
{
	std::vector<cv::Mat> srcimages;
	cv::Mat convertToUC1(cv::Mat src)
	{
		if (src.type() != CV_16UC1)
			return cv::Mat();
		else if (src.type() == CV_8UC1)
			return src;
		double maxx = 0, minn = 0;
		double* max = &maxx;
		double* min = &minn;
		cv::minMaxIdx(src, min, max);
		//��ʾԭʼͼ��
		cv::Mat ret = src.clone();
		for (int i = 0; i < src.rows; i++) {
			for (int j = 0; j < src.cols; j++) {
				ret.at<short>(i, j) = 255 * (src.at<short>(i, j) - minn) * 1 / (maxx - minn);
			}
		}
		ret.convertTo(ret, CV_8UC1);
		return ret;
	}
	void averageImages(std::vector<cv::Mat> src, cv::Mat& result)
	{
		if (src.size() == 0)
		{
			std::cout << "averageImages src size=0";
			return;
		}
		cv::Mat first = src[0];
		for (int i = 0; i < first.rows; ++i)
		{
			for (int j = 0; j < first.cols; ++j)
			{
				for (int k = 0; k < src.size(); ++k)
					result.at<double>(i, j) += srcimages[k].at<uchar>(i, j);
			}
		}
	}
	//TPS��������rΪ��������ƽ��
	double tps_basis(double r)
	{
		if (r == 0.0)
		{
			return 0.0;
		}
		else
		{
			return (r * log(r));
		}
	}

	//n*n����
	void cal_K(std::vector<cv::Point2f> P, cv::Mat& K)
	{
		if (K.empty())
		{
			K.create(P.size(), P.size(), CV_32FC1);
		}
		else if (K.rows != P.size() || K.cols != P.size())
		{
			resize(K, K, cv::Size(P.size(), P.size()));
		}

		for (int i = 0; i < P.size(); i++)
		{
			for (int j = i; j < P.size(); j++)
			{
				cv::Point2f diff_p = P[i] - P[j];
				double diff = diff_p.x * diff_p.x + diff_p.y * diff_p.y;
				float U = (float)tps_basis(diff);
				K.ptr<float>(i)[j] = U;
				K.ptr<float>(j)[i] = U;
			}
		}
	}

	//(n+3)*(n+3)����
	void cal_L(std::vector<cv::Point2f> points1, cv::Mat& L)
	{
		int N = points1.size();

		cv::Mat L_tmp = cv::Mat::zeros(cv::Size(N + 3, N + 3), CV_32FC1);

		cv::Mat P = cv::Mat::ones(cv::Size(3, N), CV_32FC1);
		for (int i = 0; i < N; i++)
		{
			P.ptr<float>(i)[1] = points1[i].x;
			P.ptr<float>(i)[2] = points1[i].y;
		}

		cv::Mat P_T;
		transpose(P, P_T);

		cv::Mat K;
		cal_K(points1, K);

		K.copyTo(L_tmp(cv::Rect(0, 0, N, N)));
		P.copyTo(L_tmp(cv::Rect(N, 0, 3, N)));
		P_T.copyTo(L_tmp(cv::Rect(0, N, N, 3)));

		L = L_tmp.clone();
	}
	//W = {w0, w1, w2, ..., a1, ax, ay}
	void cal_W(std::vector<cv::Point2f> points1, std::vector<cv::Point2f> points2, cv::Mat& W)
	{
		int N = points1.size();
		cv::Mat L;
		cal_L(points1, L);

		cv::Mat Y = cv::Mat::zeros(cv::Size(2, N + 3), CV_32FC1);  //row:N+3, col:2
		for (int i = 0; i < N; i++)
		{
			Y.ptr<float>(i)[0] = points2[i].x;
			Y.ptr<float>(i)[1] = points2[i].y;
		}

		solve(L, Y, W, cv::DECOMP_LU);   //LW = Y,��W
	}

	cv::Point2f Tps_Transformation(std::vector<cv::Point2f> points1, cv::Point2f point, cv::Mat W)
	{

		cv::Point2f out;

		float a1_x = W.at<float>(W.rows - 3, 0);
		float ax_x = W.at<float>(W.rows - 2, 0);
		float ay_x = W.at<float>(W.rows - 1, 0);

		float a1_y = W.at<float>(W.rows - 3, 1);
		float ax_y = W.at<float>(W.rows - 2, 1);
		float ay_y = W.at<float>(W.rows - 1, 1);

		float affine_x = a1_x + ax_x * point.x + ay_x * point.y;
		float affine_y = a1_y + ax_y * point.x + ay_y * point.y;

		float nonrigid_x = 0;
		float nonrigid_y = 0;

		for (int j = 0; j < points1.size(); j++)
		{
			cv::Point2f diff_p = points1[j] - point;
			double diff = diff_p.x * diff_p.x + diff_p.y * diff_p.y;
			double tps_diff = tps_basis(diff);
			nonrigid_x += W.at<float>(j, 0) * tps_diff;
			nonrigid_y += W.at<float>(j, 1) * tps_diff;
		}

		out.x = affine_x + nonrigid_x;
		out.y = affine_y + nonrigid_y;

		return out;   //����(x, y)��Ӧ�ĵ�����(x', y')
	}
	void Tps_TxTy(std::vector<cv::Point2f> points1, std::vector<cv::Point2f> points2, int row, int col, cv::Mat& Tx, cv::Mat& Ty)
	{

		cv::Mat mapX(row, col, CV_32FC1);
		cv::Mat mapY(row, col, CV_32FC1);


		cv::Mat W;
		cal_W(points1, points2, W);

		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				cv::Point2f pt = Tps_Transformation(points1, cv::Point2f(float(j), float(i)), W);
				mapX.at<float>(i, j) = pt.x;
				mapY.at<float>(i, j) = pt.y;
			}
		}

		mapX.copyTo(Tx);
		mapY.copyTo(Ty);
	}

	cv::Mat threshold(cv::Mat src)
	{
		/*cv::Mat result = src.clone();
		cv::threshold(src, result, 50, 255, cv::THRESH_BINARY);
		return result;*/
		double maxx = 0, minn = 0;
		double* max = &maxx;
		double* min = &minn;
		cv::Mat result = src.clone();
		cv::minMaxIdx(result, min, max);
		while (1) {
			double mid = (*min + *max) / 2;
			cv::threshold(result, result, mid, 255, cv::THRESH_BINARY);
			cv::minMaxIdx(result, min, max);
			double newmid = (*min + *max) / 2;
			if (std::abs(newmid - mid) < 10)
				break;
		}
		return result;
	}

	void Algorithm::init()
	{
		if (inited)
			return;
		CT_Image *ctimage = &uStatus::g_ctimage;
		if (ctimage->getNumberOfImages() == 0)
			return;
		int* dims = ctimage->getCTImageDataVtk(0)->GetDimensions();
		double spacing[3];
		ctimage->getCTImageDataVtk(0)->GetSpacing(spacing);
		for (int i = 0; i < ctimage->getNumberOfImages(); ++i)
		{
			cv::Mat tmp(dims[0], dims[1], CV_16UC1, ctimage->getCTImageDataVtk(i)->GetScalarPointer());
			cv::Mat distMat;
			cv::Size dsize;
			dsize.width = dims[0] * spacing[0];
			dsize.height = dims[1] * spacing[1];
			cv::resize(tmp, distMat, dsize);
			srcimages.push_back(convertToUC1(distMat).clone());
		}
		inited = 1;
	}

	void Algorithm::doRegister()
	{
		if (!inited)
			init();
		cv::Mat _8uc1_refimg = srcimages[uStatus::g_ctimage.getNumberOfImages()/2];
		qDebug() << QDateTime::currentDateTime();
		QFuture<void> f=QtConcurrent::run(this,&Algorithm::_register, _8uc1_refimg);
		QFutureWatcher<void> wa;
		wa.setFuture(f);
		connect(&wa, &QFutureWatcher<void>::finished, this, &Algorithm::signal_register_finished);
		QEventLoop lp;
		connect(&wa, &QFutureWatcher<void>::finished, &lp, &QEventLoop::quit);
		lp.exec();
		qDebug() << QDateTime::currentDateTime();
	}

	void Algorithm::doSegment(int th)
	{
		/*cv::Mat _8uc1_refimg = srcimages[uStatus::g_ctimage.getNumberOfImages() / 2];
		cv::Mat average = cv::Mat::zeros(_8uc1_refimg.size(), CV_64FC1);
		averageImages(srcimages, average);

		double maxx = 0, minn = 0;
		double* max = &maxx;
		double* min = &minn;

		cv::minMaxIdx(average, min, max);
		for (int i = 0; i < average.rows; i++) {
			for (int j = 0; j < average.cols; j++) {
				average.at<double>(i, j) = 255 * (average.at<double>(i, j) - minn) * 1 / (maxx - minn);
			}
		}
		average.convertTo(average, CV_8UC1);*/
		if (!inited)
			return;
		for(int mm=0;mm<uStatus::g_ctimage.getNumberOfImages();++mm)
		{
			cv::Mat src = srcimages[mm].clone();
			//cv::flip(src, src, 0);
			cv::Mat _threshold;
			if (th == 0)
			{
				_threshold = threshold(src);
			}
			else
			{
				cv::threshold(src, _threshold, th, 255, cv::THRESH_BINARY);
			}
			//������
			cv::Mat closed = _threshold.clone();
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
			morphologyEx(_threshold, closed, cv::MORPH_CLOSE, kernel);

			cv::Mat labels, stats, centroids;
			int nccomps = cv::connectedComponentsWithStats(
				closed,
				labels,
				stats,
				centroids
			);
			std::vector<cv::Vec3b> color(nccomps + 1);
			color[0] = cv::Vec3b(0, 0, 0);//����ɫ
			for (int m = 1; m <= nccomps; m++) {
				color[m] = cv::Vec3b(rand() % 256, rand() % 256, rand() % 256);
			}
			cv::Mat src_color = cv::Mat::zeros(closed.size(), CV_8UC3);
			for (int x = 0; x < closed.rows; x++)
				for (int y = 0; y < closed.cols; y++)
				{
					int label = labels.at<int>(x, y);//ע��labels��int�ͣ�����uchar.
					src_color.at<cv::Vec3b>(x, y) = color[label];
				}

			cv::Mat connected(_threshold.size(), CV_8UC1);
			cv::cvtColor(src_color, connected, cv::COLOR_BGR2GRAY);
			cv::threshold(connected, connected, 0, 255, cv::THRESH_BINARY);
			//cv::imshow("connected", connected);

			// Floodfill from point (0, 0) �Ե�(0,0)Ϊ���ӵ㣬������ˮ���
			cv::Mat im_floodfill = connected.clone();
			floodFill(im_floodfill, cv::Point(0, 0), cv::Scalar(255));
			//cv::imshow("im_floodfill", im_floodfill);
			cv::Mat im_floodfill_inv;
			bitwise_not(im_floodfill, im_floodfill_inv);
			// Combine the two images to get the foreground. ���ǰ��
			cv::Mat fill = (connected | im_floodfill_inv);
			cv::Mat lungmask = fill - connected;
			//cv::imshow("lung_mask", lungmask);

			cv::Mat cannyResult = lungmask.clone();
			cv::Canny(lungmask, cannyResult, 20, 80, 3, false);
			//cv::imshow("canny", cannyResult);

			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierarchy;
			findContours(cannyResult, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

			int maxContourSize = 0;
			int maxContourIndex = -1;
			cv::Mat contourImage = src.clone();
			for (int i = 0; i < contours.size(); i++)
			{
				if (contours[i].size() > 20)
				{
					cv::Scalar contourColor = 255;
					drawContours(contourImage, contours, i, contourColor, 2, 8, hierarchy, 0, cv::Point(0, 0));
				}

			}

			//cv::imshow("result-draw-contour", contourImage);

			cv::Mat result(contourImage.size(), CV_8UC1);
			for (int x = 0; x < result.rows; x++)
				for (int y = 0; y < result.cols; y++)
				{
					uchar v = lungmask.at<uchar>(x, y) == 255 ? 1 : 0;
					result.at<uchar>(x, y) = contourImage.at<uchar>(x, y) * v;
				}
			cv::Mat redImage(result.rows, result.cols, CV_8UC3, cv::Scalar(0, 0, 0));
			for (int i = 0; i < result.rows; ++i) {
				for (int j = 0; j < result.cols; ++j) {
					uchar grayValue = result.at<uchar>(i, j);
					if(grayValue>0)
						redImage.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 255);
				}
			}
			vtkSmartPointer<vtkImageData> ret = vtkSmartPointer<vtkImageData>::New();
			vtkSmartPointer<vtkInformation> in = vtkSmartPointer<vtkInformation>::New();
			ret->SetScalarType(VTK_UNSIGNED_CHAR, in);
			ret->SetSpacing(1, 1, 1);
			ret->SetOrigin(0, 0, 0);
			ret->SetDimensions(redImage.cols, redImage.rows, 1);
			ret->SetNumberOfScalarComponents(3, in);
			ret->AllocateScalars(in);
			uchar* pixelsPointer = (uchar *)ret->GetScalarPointer();
			for (int i = 0; i < redImage.rows; i++) {
				for (int j = 0; j < redImage.cols; j++) {
					cv::Vec3b color = redImage.at<cv::Vec3b>(i, j);
					for (int c = 0; c < 3; ++c) {
						pixelsPointer[3 * (i * redImage.cols + j) + 2-c] = color[c];
					}
				}
			}
			ret->Modified();
			//cv::imshow("result", result);
			//cv::waitKey();
			uStatus::g_ctimage.getCTImageDataVtk_Segmented(mm)->DeepCopy(ret);
		}
	}

	void Algorithm::_register(cv::Mat refimage)
	{
		QProgressDialog* progressDialog = createProgressDialog(tr("Registering"), tr("do Register, Please Wait..."), 101);
		progressDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
		cv::Mat img0_e = refimage.clone(), imgi_e;
		equalizeHist(refimage, img0_e);  //�Բο�ͼ�����ֱ��ͼ���⻯
		std::vector<cv::Point2f> p1;
		progressDialog->setValue(1);
		//���Shi-Tomasi�ǵ�
		goodFeaturesToTrack(img0_e, p1, 200, 0.015, 20.0, cv::Mat(), 3, false, 0.04);    //��ȡshi-tomasi�ǵ�
		std::vector<cv::Point2f> p2 = p1;
		std::vector<uchar> status;
		std::vector<float> err;         //���ٹ����еĴ���
		cv::TermCriteria termcrit(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 35, 0.01);
		cv::Size winSize(51, 51);
		progressDialog->setValue(2);
		int step = 5;
		int progress{0};
		std::vector<vtkSmartPointer<vtkImageData>> vimages;
		for (int i = 0; i < srcimages.size(); i++)  //�ֱ���׼����ͼ��
		{
			++progress;
			if(progressDialog->value()<100 && progress%step==0)
			{
				progressDialog->setValue(progressDialog->value() + 1);
			}
			cv::Mat imgi = srcimages[i];
			equalizeHist(imgi, imgi_e);  //�Ը���ͼ�����ֱ��ͼ���⻯

			//���ٲο�ͼ���Ͻǵ��ڸ���ͼ���ϵĶ�Ӧλ��
			calcOpticalFlowPyrLK(img0_e, imgi_e, p1, p2, status, err, winSize, 3, termcrit, cv::OPTFLOW_USE_INITIAL_FLOW, 0.001);

			std::vector<uchar> RANSACStatus;//���Ա��ÿһ��ƥ����״̬������0��Ϊ��㣬����1��Ϊ�ڵ㡣
			findFundamentalMat(p1, p2, RANSACStatus, cv::FM_RANSAC, 3);//p1 p2����Ϊfloat��
			std::vector<cv::Point2f> f1_features_ok;
			std::vector<cv::Point2f> f2_features_ok;
			for (int k = 0; k < p1.size(); k++)   //�޳�����ʧ�ܵ�
			{
				if (status[k] && RANSACStatus[k])  //���׷�ٳɹ���������RANSAC����������Ϊ��ƥ����ʹ׼ȷ�ģ��������޳�
				{
					f1_features_ok.push_back(p1[k]);       //��׼ͼ������
					f2_features_ok.push_back(p2[k]);     //����ͼ������
				}
			}

			cv::Mat out;
			cv::Mat Tx, Ty;
			//��׼ȷ��ƥ��������TPS�任����ȡ����ӳ��
			Tps_TxTy(f1_features_ok, f2_features_ok, imgi.rows, imgi.cols, Tx, Ty);
			remap(imgi, out, Tx, Ty, cv::INTER_CUBIC);  //�����ز���

			//ʹ�õ�ǰ֡�ĸ�����Ϣ��������һ֡ͼ��ĳ�ʼ���ٵ�
			for (int k = 0; k < p1.size(); k++)
			{
				p2[k] = p2[k] + p2[k] - p1[k];
			}
			srcimages[i] = out.clone(); 
			vtkSmartPointer<vtkImageData> ret = vtkSmartPointer<vtkImageData>::New();
			vtkSmartPointer<vtkInformation> in = vtkSmartPointer<vtkInformation>::New();
			ret->SetScalarType(VTK_UNSIGNED_CHAR, in);
			ret->SetSpacing(1,1,1);
			ret->SetOrigin(0,0,0);
			ret->SetDimensions(out.cols,out.rows,1);
			//ret->SetExtent(0, out.cols - 1, 0, out.rows - 1, 0, 0);
			ret->SetNumberOfScalarComponents(1, in);
			ret->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
			for (int i = 0; i < out.rows; i++) {
				for (int j = 0; j < out.cols; j++) {
					uchar *v=(uchar *)ret->GetScalarPointer(j,i,0);
					*v = out.at<uchar>(i, j);
				}
			}
			ret->Modified();

			vimages.push_back(ret);
		}
		uStatus::g_ctimage.setRegisteredImages(vimages);
		progressDialog->setValue(100);
		progressDialog->deleteLater();
	}
}


