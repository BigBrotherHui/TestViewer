#pragma once
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>
#include <vtkHandleWidget.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkLineSource.h>
class vtkReslicePlaneCursorCallback;

namespace asclepios::gui
{
	class vtkReslicePlaneCursorWidget final : public vtkAbstractWidget
	{
	public:

		static vtkReslicePlaneCursorWidget* New();
		vtkTypeMacro(vtkReslicePlaneCursorWidget, vtkAbstractWidget);

		vtkReslicePlaneCursorWidget();
		~vtkReslicePlaneCursorWidget();

		void CreateDefaultRepresentation() override;
		void SetEnabled(int) override;
		void setPlane(int t_plane);
		void SetCursor(int) override;
		void setCursorPosition(double* t_position) const;
		void setCursorCenterPosition(const double* t_position);
		double* getCursorCenterPosition();
		    

	private:
		vtkSmartPointer<vtkHandleWidget> m_centerMovementWidget = {};
		vtkSmartPointer<vtkReslicePlaneCursorCallback> m_centerMovementCallback = {};

		void initializeWidget();

		enum widgetState { start = 0,rotate, translate,bothTranslate, thickness_horizontal, thickness_vertical,expand };

	protected:
		int m_state = 0;
		int m_plane = -1;
		double lastCursorPos[3]{};
		double m_cursorCenterPosition[3]{};
                static double __lastCursorPos[3];
		void startWidgetInteraction(int handleNum);
		void widgetInteraction(int handleNum);
		void endWidgetInteraction(int handleNum);

		static void leftMouseDownAction(vtkAbstractWidget* w);
		static void moveMouse(vtkAbstractWidget* w);
		static void leftMouseUpAction(vtkAbstractWidget* w);
		void rotateCursor(double t_angle) const;
                void translateCursor(double x, double y,double z, char moveAxes) const;
                void expandWall(double x, double y, double z, unsigned char moveAxes) const;
                void pickCurrentSlice(double x, double y, double z, unsigned char moveaxes);
		friend class vtkReslicePlaneCursorCallback;
                int m_selectedAxis{-1};
	private:
		bool isRotating{false};
	};
}
