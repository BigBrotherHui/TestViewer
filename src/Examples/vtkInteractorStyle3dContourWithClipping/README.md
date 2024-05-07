# vtkInteractorStyle3dContourWithClipping

it is an interactorStyle for 3D rendering.

placing a node on the surface of the object is possible with this interactorStyle.
the contour representation of the contour widget is the the geodesic path of the object.
a looped tube will be generated from the downsampled geodesic path.
all the polydata inside or on the tube will be removed which means clipping the object by the tube.
the object will become 2 disconneted object by using vtkPolyDataConnectivityFilter.
