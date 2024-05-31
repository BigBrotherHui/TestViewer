#include "globalsignal.h"

void GlobalSignal::slot_sliceParamsChanged(int slicecount, int thickness, double spacing)
{
    m_sliceCount = slicecount;
    m_thickness = thickness;
    m_spacing = spacing;
    emit signal_sliceParamsChanged(slicecount, thickness, spacing);
}

GlobalSignal::GlobalSignal(QObject *parent) : QObject(parent)
{

}
