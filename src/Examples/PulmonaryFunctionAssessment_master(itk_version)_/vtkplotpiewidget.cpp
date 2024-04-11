#include "vtkplotpiewidget.h"
#include <vtkFloatArray.h>
#include <vtkDataObject.h>
#include <vtkNamedColors.h>
#include <vtkFieldData.h>
#include <vtkProperty2D.h>
#include <vtkTextProperty.h>
#include <vtkLegendBoxActor.h>
#include <vtkColorSeries.h>
VtkPlotPieWidget::VtkPlotPieWidget(QWidget* parent) : QVTKOpenGLNativeWidget(parent)
{
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);
    setRenderWindow(renderWindow);
    actor = vtkSmartPointer<VtkPieChartActor>::New();
    actor->SetTitle("Proportion");
    actor->GetPositionCoordinate()->SetValue(0.05, 0.1, 0.0);
    actor->GetPosition2Coordinate()->SetValue(0.95, 0.85, 0.0);
    actor->GetProperty()->SetColor(0, 0, 0);
    actor->GetProperty()->SetLineWidth(2);
    actor->GetLabelTextProperty()->SetFontSize(18);
    actor->LegendVisibilityOn();

    vtkNew<vtkNamedColors> colors;
    actor->GetTitleTextProperty()->SetColor(
        colors->GetColor3d("Banana").GetData());
    actor->GetTitleTextProperty()->SetFontSize(40);
    actor->GetLabelTextProperty()->SetColor(
        colors->GetColor3d("Bisque").GetData());
    actor->GetLabelTextProperty()->SetFontSize(24);

    renderer->SetBackground(1,1,1);
    renderer->AddActor(actor);
}

VtkPlotPieWidget::~VtkPlotPieWidget()
{
    
}

void VtkPlotPieWidget::setData(const std::map<std::string, int>& data)
{
    int numTuples = static_cast<int>(data.size());

    vtkNew<vtkFloatArray> bitter;
    bitter->SetNumberOfTuples(numTuples);

    std::map<std::string, int>::const_iterator m;
    int i = 0;
    int total = 0;
    for (m = data.begin(); m != data.end(); ++m)
    {
        bitter->SetTuple1(i++, m->second);
        total += m->second;
    }

    vtkNew<vtkDataObject> dobj;
    dobj->GetFieldData()->AddArray(bitter);

    actor->SetInputData(dobj);
    actor->GetLegendActor()->SetNumberOfEntries(numTuples);

    vtkNew<vtkColorSeries> colorSeries;
    colorSeries->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_PASTEL2);
    i = 0;
    for (m = data.begin(); m != data.end(); ++m)
    {
        vtkColor3ub rgb = colorSeries->GetColorRepeating(i);
        actor->SetPieceColor(i, static_cast<double>(rgb.GetRed()) / 255.0,
            static_cast<double>(rgb.GetGreen()) / 255.0,
            static_cast<double>(rgb.GetBlue()) / 255.0);
        QString t = QString("%2%").arg(QString::number(m->second*1./total*100,'f',2));
        actor->SetPieceLabel(i,t.toStdString().c_str());
        actor->GetLegendActor()->SetEntryString(i, m->first.c_str());
        i++;
    }
}
