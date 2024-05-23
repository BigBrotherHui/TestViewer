#pragma once

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransformFilter.h>
#include <vtkContourFilter.h>
#include <vtkAssembly.h>
namespace asclepios::gui
{
	class vtkResliceActor final : public vtkObject
	{
	public:
		static vtkResliceActor* New();
		vtkTypeMacro(vtkResliceActor, vtkObject);
		vtkResliceActor() { createActor(); }
		~vtkResliceActor() = default;

		//getters
                [[nodiscard]] vtkActor* getActorTranslate() const { return m_actorTranslate; }
                [[nodiscard]] vtkActor* getActorRotate() const { return m_actorRotate; }
                vtkAssembly* getActorText() const { return m_actorText; }
                vtkSmartPointer<vtkAppendPolyData> append;
		[[nodiscard]] vtkMTimeType getLineTime() const { return m_cursorLines[0]->GetMTime(); }

		//setters
		void setCameraDistance(const double t_distance) { m_cameraDistance = t_distance; }
		void setDisplaySize(const double* t_size);
		void setDisplayOriginPoint(const double* t_point);
		void setCenterPosition(const double* t_center);
                double* getCenterPosition();
		void createActor();
		void update();
		void reset() const;
		void createColors(double* t_color1, double* t_color2);
                void createWallRepresentation(double x,double y,double z,int value=0);
                double getWallSpacing()
                { return m_wallSpacing;
                }
                int getImageNumFront()
                { return m_imageNumFront; 
                }
                int getImageNumBack()
                { return m_imageNumBack;
                }
                double getActorScale()
                { return actorScale;
                }
	private:
                vtkSmartPointer<vtkAppendPolyData> m_appenderTranslate = {}, m_appenderRotate = {};
                vtkSmartPointer<vtkActor> m_actorRotate = {}, m_actorTranslate = {};
		vtkSmartPointer<vtkTransformFilter> m_filter = {};
                vtkSmartPointer<vtkLineSource> m_cursorLines[2] = {}, m_cursorLines2[4];
		vtkSmartPointer<vtkUnsignedCharArray> m_colors[3] = {};
		double m_windowSize[3] = {};
		double m_windowOrigin[3] = {};
		double m_centerPointDisplayPosition[3] = {};
		double m_cameraDistance = 0;
		int m_start = 0;
                vtkSmartPointer<vtkPolyData> arrowTop1=vtkSmartPointer<vtkPolyData>::New();
                vtkSmartPointer<vtkPolyData> arrowTop2 = vtkSmartPointer<vtkPolyData>::New();
                vtkSmartPointer<vtkPolyData> arrowLeft1 = vtkSmartPointer<vtkPolyData>::New();
                vtkSmartPointer<vtkPolyData> arrowLeft2 = vtkSmartPointer<vtkPolyData>::New();
                vtkSmartPointer<vtkPolyData> polydataWall = vtkSmartPointer<vtkPolyData>::New();
                double m_wallSpacing{1};
                int m_imageNumFront{10};
                int m_imageNumBack{10};
                double actorScale = 5;
                vtkSmartPointer<vtkAssembly> m_actorText{nullptr};
	};
}
