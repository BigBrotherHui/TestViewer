#pragma once

#include "PointHandleRepresentation3D.h"
#include <vtkSmartPointer.h>
#include <vtkWidgetRepresentation.h>
#include "vtkresliceactor.h"
#include "SphereHandleRepresentation.h"

namespace asclepios::gui
{
	class vtkResliceWidgetRepresentation final : public vtkWidgetRepresentation
	{
	public:
		static vtkResliceWidgetRepresentation* New();
		vtkTypeMacro(vtkResliceWidgetRepresentation, vtkWidgetRepresentation);
		vtkResliceWidgetRepresentation();
		~vtkResliceWidgetRepresentation() = default;
		void reset();
		void instantiateHandleRepresentation();
		void StartWidgetInteraction(double eventPos[2]) override {};
		void WidgetInteraction(double newEventPos[2]) override {};

		//�����vtkAbstractWidget�н���SetEnabled��1���е���
		//��vtkWidgetRepresentation�޵��ã��е�rep������RenderOpaqueGeometry��RenderTranslucentPolygonalGeometry��HasTranslucentPolygonalGeometry
		//��һ��ʼ��������
		void BuildRepresentation() override;
		void ReleaseGraphicsResources(vtkWindow* w) override;
		int RenderOverlay(vtkViewport* viewport) override;
		//����actor�������������actor��vtkViewport�в��ɼ���vtkViewport����vtkRenderer�ĸ��ࣩ
		int RenderOpaqueGeometry(vtkViewport* viewport) override;
		int HasTranslucentPolygonalGeometry() override;


		void rotate(double t_angle);
                void translate(double x, double y,double z, char moveAxes);
                void expand(double x, double y, double z, unsigned char moveAxes);
                void pickCurrentSlice(double x, double y, double z, unsigned char moveaxes);
		void setPlane(int t_plane);

		void setCursorPosition(double* t_position);
		void SetVisibility(int _arg) override;
		int GetVisibility() override;
		int ComputeInteractionState(int X, int Y, int modify) override;

		[[nodiscard]] vtkSmartPointer<vtkResliceActor> getResliceActor() const { return m_cursorActor; }
                [[nodiscard]] vtkSmartPointer<SphereHandleRepresentation> getCenterMovementRepresentation() const
                {
			return m_centerMovementPointRepresentation;
		}
		enum InteractionState { outside = 0, translateCursorHor,translateCursorVer,rotateCursor, mipCursor, handleCursor };
	private:
                vtkSmartPointer<SphereHandleRepresentation> m_centerMovementPointRepresentation = {};
		vtkSmartPointer<vtkResliceActor> m_cursorActor;
		bool m_cursorVisibility = false;
		double m_rotationAngle = 0;
		int m_plane = -1;
		void initializeRepresentation();
		void createCursor();
	};
}
