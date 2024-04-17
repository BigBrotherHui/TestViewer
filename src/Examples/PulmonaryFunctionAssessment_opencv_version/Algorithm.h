#pragma once

#include "opencv2/opencv.hpp"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <QObject>
namespace ImageProcess
{
	class Algorithm : public QObject
	{
		Q_OBJECT
	public:
		void init();
		void doRegister();
		void doSegment(int threshold=0);
	signals:
		void signal_register_finished();
	protected:
		void _register(cv::Mat);
	private:
		bool inited{ false };
		std::vector<vtkSmartPointer<vtkImageData>> mImages;
	};
}
