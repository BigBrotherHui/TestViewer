#ifndef GLOBALSIGNAL_H
#define GLOBALSIGNAL_H

#include <QObject>

class GlobalSignal : public QObject
{
    Q_OBJECT
public:
    static GlobalSignal& instance()
    {
        static GlobalSignal signal;
        return signal;
    }
    int sliceCount()
    { return m_sliceCount;
    }
    int thickness()
    { return m_thickness;
    }
    double spacing()
    { return m_spacing;
    }
public slots:
    void slot_sliceParamsChanged(int slicecount, double thickness, double spacing);
signals:
    void signal_sliceParamsChanged(int slicecount, double thickness, double spacing);

protected:
    GlobalSignal(QObject *parent=nullptr);
private:
    int m_sliceCount;
    double m_thickness;
    double m_spacing;
};

#endif // GLOBALSIGNAL_H
