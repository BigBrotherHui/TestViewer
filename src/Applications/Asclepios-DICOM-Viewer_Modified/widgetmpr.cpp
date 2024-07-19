#include "widgetmpr.h"
#include <QFocusEvent>
#include <vtkEventQtSlotConnect.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include "tabwidget.h"
#include "globalsignal.h"
#include "vtkreslicewidgetrepresentation.h"
#include "LatticeWidget.h"

asclepios::gui::WidgetMPR::WidgetMPR(QWidget* parent)
	: WidgetBase(parent)
{
	initData();
	initView();
	createConnections();
	m_tabWidget = parent;
        connect(&GlobalSignal::instance(), &GlobalSignal::signal_sliceParamsChanged, this,
                [&](int slicecount, int thickness, double spacing) {
                    if (slicecount <= 0) return;
					int index=m_widgetMPR->getActiveRenderWindowIndex();
                    auto rep=dynamic_cast<vtkResliceWidgetRepresentation*>(
                        m_widgetMPR->getResliceWidget()->getReslicePlaneCursorWidget(index)->GetRepresentation());
                    auto actor=rep->getResliceActor();
                    int frontSliceCount{0}, backSliceCount{0};
                    if (slicecount % 2 == 0) {
                        frontSliceCount = slicecount / 2;
                        backSliceCount = slicecount / 2-1;
                    }
                    else
                    {
                        slicecount = slicecount - 1;
                        frontSliceCount = slicecount / 2;
                        backSliceCount = slicecount / 2;
                    }
                    actor->setImageNumFront(frontSliceCount);
                    actor->setImageNumBack(backSliceCount);
                    actor->setWallSpacing(spacing/actor->getActorScale());
					actor->setSlabSliceCount(thickness);
                    actor->createWallRepresentation(0, 0, 0,0,actor->getPickedSlice());
					m_widgetMPR->updateWallCommand();
                    m_widgetMPR->getResliceWidget()->getReslicePlaneCursorWidget(index)->Render();
        });
        for (int i=0;i<3;++i)
            connect(m_widgetMPR->getLatticeWidget()->getLatticeResliceWidget(i),
                    &LatticeResliceWidget::signal_wallChanged, this,
                    [&](bool isUp) { 
			int index = m_widgetMPR->getActiveRenderWindowIndex();
			 if (m_widgetMPR->getLatticeWidget()->getLatticeResliceWidget(0) == sender()) {
                            auto rep = dynamic_cast<vtkResliceWidgetRepresentation*>(
                                m_widgetMPR->getResliceWidget()->getReslicePlaneCursorWidget(index)->GetRepresentation());
                            auto actor = rep->getResliceActor();
                            int frontImageNum = actor->getImageNumFront();
                            if (isUp)
                                frontImageNum += 1;
                            else
                                frontImageNum -= 1;
                            if (frontImageNum < 1) frontImageNum = 1;
                            actor->createWallRepresentation(
                                0, -frontImageNum * actor->getActorScale() * actor->getWallSpacing(), 0, 12,
                                actor->getPickedSlice());
                            actor->setImageNumFront(frontImageNum);
                            m_widgetMPR->getResliceWidget()->getReslicePlaneCursorWidget(index)->Render();
                            m_widgetMPR->getResliceWidget()->InvokeEvent(cursorFinishMovement);
			 }
                         else if (m_widgetMPR->getLatticeWidget()->getLatticeResliceWidget(1) == sender())
                        {
                             auto rep =
                                 dynamic_cast<vtkResliceWidgetRepresentation*>(m_widgetMPR->getResliceWidget()
                                                                                   ->getReslicePlaneCursorWidget(index)
                                                                                   ->GetRepresentation());
                             auto actor = rep->getResliceActor();
                             if (isUp)
                                 actor->setPickedSlice(actor->getPickedSlice() + 1);
                             else
                                 actor->setPickedSlice(actor->getPickedSlice() - 1);
                             actor->createWallRepresentation(0, 0, 0, 0, actor->getPickedSlice());
                             m_widgetMPR->getResliceWidget()->getReslicePlaneCursorWidget(index)->Render();
                             m_widgetMPR->getResliceWidget()->InvokeEvent(cursorFinishMovement);
                        }
                        else if (m_widgetMPR->getLatticeWidget()->getLatticeResliceWidget(2) == sender()) {
                            auto rep =
                                dynamic_cast<vtkResliceWidgetRepresentation*>(m_widgetMPR->getResliceWidget()
                                                                                  ->getReslicePlaneCursorWidget(index)
                                                                                  ->GetRepresentation());
                            auto actor = rep->getResliceActor();
                            int backImageNum = actor->getImageNumBack();
                            if (isUp)
                                backImageNum += 1;
                            else
                                backImageNum -= 1;
    			    if (backImageNum < 1) backImageNum = 1;
                            actor->createWallRepresentation(
                                0, backImageNum * actor->getActorScale() * actor->getWallSpacing(), 0, 11,
                                actor->getPickedSlice());
                            actor->setImageNumBack(backImageNum);
                            m_widgetMPR->getResliceWidget()->getReslicePlaneCursorWidget(index)->Render();
                            m_widgetMPR->getResliceWidget()->InvokeEvent(cursorFinishMovement);
                        }
        });
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::render()
{
	if (!m_image || !m_widgetMPR)
	{
		return;
	}
	m_toolbar->getUI().toolButtonReslicer->setVisible(false);
	m_toolbar->getUI().toolButtonReset->setVisible(false);
	startLoadingAnimation();
	if (m_image->getIsMultiFrame())
	{
		m_widgetMPR->setImage(m_image);
	}
	else
	{
		m_widgetMPR->setImage(m_image);
		m_widgetMPR->setSeries(m_series);
	}
	m_future = QtConcurrent::run(onRenderAsync, this);
	Q_UNUSED(connect(this, &WidgetMPR::finishedRenderAsync,
		this, &WidgetMPR::onFinishedRenderAsync));
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::onActivateResliceWidget(const bool& t_flag) const
{
	if (m_widgetMPR)
	{
		m_widgetMPR->setShowCursor(t_flag);
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::onResetResliceWidget() const
{
	if (m_widgetMPR)
	{
		m_widgetMPR->resetResliceWidget();
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::onSetMaximized() const
{
	if (m_tabWidget)
	{
		dynamic_cast<TabWidget*>
			(m_tabWidget)->onMaximize();
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::onActivateWidget(const bool& t_flag, QObject* t_object)
{
	for (const auto& widget : m_qtvtkWidgets)
	{
		if (widget == t_object)
		{
			m_widgetMPR->setActiveRenderWindow(widget->GetRenderWindow());
		}
	}
	if (t_flag)
	{
		auto* event = new QFocusEvent(QEvent::FocusIn,
			Qt::FocusReason::MouseFocusReason);
		focusInEvent(event);
		delete event;
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::onFinishedRenderAsync()
{
	stopLoadingAnimation();
	for (const auto& widget : m_qtvtkWidgets)
	{
		widget->setVisible(true);
	}
	m_widgetMPR->setRenderWindowsMPR(
		m_qtvtkWidgets[0]->GetRenderWindow(),
		m_qtvtkWidgets[1]->GetRenderWindow(),
		m_qtvtkWidgets[2]->GetRenderWindow());
	m_widgetMPR->render();
	m_toolbar->getUI().toolButtonReslicer->setVisible(true);
	m_toolbar->getUI().toolButtonReset->setVisible(true);
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::initData()
{
	m_toolbar = new ToolbarWidgetMPR(this);
	for (auto i = 0; i < 3; ++i)
	{
		m_qtvtkWidgets[i] = new QVTKOpenGLNativeWidget(this);
		m_renderWindow[i] = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
		m_qtvtkWidgets[i]->SetRenderWindow(m_renderWindow[i]);
		m_qtvtkWidgets[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		m_qtvtkWidgets[i]->setVisible(false);
	}
	m_widgetMPR = std::make_unique<vtkWidgetMPR>();
	m_vtkEvents = std::make_unique<vtkEventFilter>(this);
	setWidgetType(WidgetType::widgetmpr);
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::initView()
{
	m_ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	auto* const layout = new QVBoxLayout(this);
	layout->setMargin(2);
	layout->setSpacing(2);
	auto* const widgetsLayout = new QGridLayout(this);
	widgetsLayout->addWidget(m_qtvtkWidgets[0], 0, 0);
	widgetsLayout->addWidget(m_qtvtkWidgets[1], 1, 0);
	widgetsLayout->addWidget(m_qtvtkWidgets[2], 0, 1, 2, 1);
	widgetsLayout->setMargin(0);
	widgetsLayout->setSpacing(2);
	dynamic_cast<QVBoxLayout*>(this->layout())->addWidget(m_toolbar);
	dynamic_cast<QVBoxLayout*>(this->layout())->addLayout(widgetsLayout);
	setLayout(layout);
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::createConnections()
{
	if (m_toolbar)
	{
		Q_UNUSED(connect(m_toolbar,
			&ToolbarWidgetMPR::activateResliceWidget,
			this, &WidgetMPR::onActivateResliceWidget));
		Q_UNUSED(connect(m_toolbar,
			&ToolbarWidgetMPR::resetResliceWidget,
			this, &WidgetMPR::onResetResliceWidget));
	}
	setFocusPolicy(Qt::FocusPolicy::WheelFocus);
	for (const auto& widgets : m_qtvtkWidgets)
	{
		widgets->installEventFilter(m_vtkEvents.get());
	}
	Q_UNUSED(connect(m_vtkEvents.get(),
		&vtkEventFilter::activateWidget,
		this, &WidgetMPR::onActivateWidget));
	Q_UNUSED(connect(m_vtkEvents.get(),
		&vtkEventFilter::setMaximized,
		this, &WidgetMPR::onSetMaximized));
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::startLoadingAnimation()
{
	m_loadingAnimation = std::make_unique<LoadingAnimation>(this);
	m_loadingAnimation->setWindowFlags(Qt::Widget);
	layout()->addWidget(m_loadingAnimation.get());
	m_loadingAnimation->show();
}

//-----------------------------------------------------------------------------
void asclepios::gui::WidgetMPR::onRenderAsync(WidgetMPR* t_self)
{
	if (t_self->m_widgetMPR)
	{
		t_self->m_widgetMPR->create3DMatrix();
		emit t_self->finishedRenderAsync();
	}
}
