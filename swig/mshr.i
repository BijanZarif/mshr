 %module mshr
 %{
    #include <numpy/arrayobject.h>
    #include <vector>

    #include <dolfin/common/types.h>
    #include <dolfin/mesh/MeshFunction.h>

    #include <mshr/CSGPrimitive.h>
    #include <mshr/CSGOperators.h>
    #include <mshr/CSGPrimitives2D.h>
    #include <mshr/CSGPrimitives3D.h>
    #include <mshr/CSGCGALDomain2D.h>
    #include <mshr/CSGCGALDomain3D.h>
    #include <mshr/TetgenMeshGenerator3D.h>

    #include <mshr/CSGGeometry.h>
    #include <mshr/CSGGeometries3D.h>

    #include <mshr/MeshGenerator.h>
    #include <mshr/CSGCGALMeshGenerator2D.h>
    #include <mshr/CSGCGALMeshGenerator3D.h>
    #include <mshr/DolfinMeshUtils.h>
    #include <mshr/Meshes.h>
 %}

%init
%{
  import_array();
%}

// Global typemaps and forward declarations
%include "dolfin/swig/typemaps/includes.i"
%include "dolfin/swig/forwarddeclarations.i"

// Global exceptions
%include <exception.i>
%include "dolfin/swig/exceptions.i"

// STL SWIG string class
%include <std_string.i>

// Local shared_ptr declarations
%include <std_shared_ptr.i>
%shared_ptr(dolfin::Variable)
%shared_ptr(dolfin::Hierarchical<dolfin::Mesh>)

%ignore dolfin::Variable::id;
%ignore dolfin::Variable::str;

%include "dolfin/swig/common/pre.i"
%import(module="dolfin") "dolfin/common/Variable.h"
%import(module="dolfin") "dolfin/common/Hierarchical.h"

%shared_ptr(dolfin::Mesh)

%ignore dolfin::Mesh::type;
%ignore dolfin::Mesh::hash;

%import "dolfin/swig/mesh/pre.i"
%import(module="dolfin") "dolfin/mesh/Mesh.h"

%ignore mshr::CSGGeometry::getType;
%ignore mshr::CSGUnion::_g0;
%ignore mshr::CSGUnion::_g1;
%ignore mshr::CSGUnion::getType;
%ignore mshr::CSGOperator::CSGOperator;

/* %ignore mshr::operator+(mshr::CSGGeometry& g0, mshr::CSGGeometry& g1); */
/* %ignore mshr::operator+(std::shared_ptr<mshr::CSGGeometry>, std::shared_ptr<mshr::CSGGeometry>); */
/* %ignore mshr::operator+(mshr::CSGGeometry&, std::shared_ptr<mshr::CSGGeometry>); */
/* %ignore mshr::operator+(std::shared_ptr<mshr::CSGGeometry>, mshr::CSGGeometry&); */

%ignore mshr::operator+;
%ignore mshr::operator-;
%ignore mshr::operator*;


%ignore mshr::CSGCGALDomain2D::operator=;
%ignore mshr::CSGCGALDomain2D::impl;
%ignore mshr::PSLG;
%ignore mshr::CSGCGALDomain3DQueryStructure::CSGCGALDomain3DQueryStructure;
%ignore mshr::CSGCGALDomain3DQueryStructure::impl;
%ignore mshr::CSGCGALDomain3D::impl;

// These are reimplemented to return numpy arrays
%ignore mshr::CSGCGALDomain3D::get_facets() const;
%ignore mshr::CSGCGALDomain3D::get_vertices() const;

%shared_ptr(mshr::CSGGeometry)
%shared_ptr(mshr::CSGPrimitive)
%shared_ptr(mshr::CSGOperator)
%shared_ptr(mshr::CSGUnion)
%shared_ptr(mshr::CSGDifference)
%shared_ptr(mshr::CSGIntersection)
%shared_ptr(mshr::CSGTranslation)
%shared_ptr(mshr::CSGScaling)
%shared_ptr(mshr::CSGRotation)
%shared_ptr(mshr::CSGPrimitive2D)
%shared_ptr(mshr::Circle)
%shared_ptr(mshr::Ellipse)
%shared_ptr(mshr::Rectangle)
%shared_ptr(mshr::Polygon)
%shared_ptr(mshr::CSGPrimitive3D)
%shared_ptr(mshr::Sphere)
%shared_ptr(mshr::Box)
%shared_ptr(mshr::Cone)
%shared_ptr(mshr::Cylinder)
%shared_ptr(mshr::Tetrahedron)
%shared_ptr(mshr::Ellipsoid)
%shared_ptr(mshr::Surface3D)
%shared_ptr(mshr::Extrude2D)
%shared_ptr(mshr::CSGCGALMeshGenerator2D)
%shared_ptr(mshr::CSGCGALMeshGenerator3D)
%shared_ptr(mshr::TetgenMeshGenerator3D)
%shared_ptr(mshr::CSGCGALDomain2D)
%shared_ptr(mshr::CSGCGALDomain3D)
%shared_ptr(mshr::UnitSphereMesh)
%shared_ptr(mshr::CSGCGALDomain3DQueryStructure)

%ignore mshr::get_boundary_mesh;

%typemap (in) std::vector<std::array<double, 3>>&
    (std::vector<std::array<double, 3>> tmp)
{
  if (!PyArray_Check($input))
  {
    SWIG_exception(SWIG_TypeError, "numpy array expected.");
  }

  PyArrayObject* array = reinterpret_cast<PyArrayObject*>($input);

  if (PyArray_TYPE(array) != NPY_DOUBLE)
  {
    SWIG_exception(SWIG_TypeError, "numpy array of doubles expected.");
  }

  npy_intp* shape = PyArray_DIMS(array);
  tmp.resize(shape[0]);
  double* in_data = static_cast<double*>(PyArray_DATA(array));
  const std::size_t size = PyArray_SIZE(array);
  double* out_data = reinterpret_cast<double*>(&tmp[0]);
  if (PyArray_ISCONTIGUOUS(array))
  {
    std::copy_n(in_data, shape[0]*shape[1], out_data);
  }
  else
  {
    const npy_intp strides = PyArray_STRIDE(array, 0)/sizeof(double);
    for (std::size_t i = 0; i < size; i++)
      out_data[i] = in_data[i*strides];
  }

  $1 = &tmp;
}

%typemap (in) std::vector<std::array<std::size_t, 3>>&
    (std::vector<std::array<std::size_t, 3>> tmp)
{
  if (!PyArray_Check($input))
  {
    SWIG_exception(SWIG_TypeError, "numpy array expected.");
  }

  PyArrayObject* array = reinterpret_cast<PyArrayObject*>($input);

  if (PyArray_TYPE(array) != NPY_INT64)
  {
    SWIG_exception(SWIG_TypeError, "numpy array of ints expected.");
  }

  npy_intp* shape = PyArray_DIMS(array);
  tmp.resize(shape[0]);
  long* in_data = static_cast<long*>(PyArray_DATA(array));

  const std::size_t size = PyArray_SIZE(array);
  std::size_t* out_data = reinterpret_cast<std::size_t*>(&tmp[0]);
  if (PyArray_ISCONTIGUOUS(array))
  {
    for (std::size_t i = 0; i < size; i++)
      out_data[i] = static_cast<std::size_t>(in_data[i]);

  }
  else
  {
    const npy_intp strides = PyArray_STRIDE(array, 0)/sizeof(std::size_t);
    for (std::size_t i = 0; i < size; i++)
      out_data[i] = static_cast<std::size_t>(in_data[i*strides]);
  }

  $1 = &tmp;
}



%include <mshr/CSGGeometry.h>
%include <mshr/CSGPrimitive.h>
%include <mshr/CSGOperators.h>
%include <mshr/CSGPrimitives2D.h>
%include <mshr/CSGPrimitives3D.h>
%include <mshr/CSGCGALDomain2D.h>
%include <mshr/CSGCGALDomain3D.h>
%include <mshr/MeshGenerator.h>
%include <mshr/CSGCGALMeshGenerator2D.h>
%include <mshr/CSGCGALMeshGenerator3D.h>
%include <mshr/TetgenMeshGenerator3D.h>
%include <mshr/CSGGeometries3D.h>
%include <mshr/DolfinMeshUtils.h>
%include <mshr/Meshes.h>

%extend mshr::CSGGeometry {
  %pythoncode %{
     def __add__(self, other) :
         if isinstance(other, dolfin.Point) :
             return CSGTranslation(self, other)
         else :
             return CSGUnion(self, other)

     def __mul__(self, other) :
         from numbers import Number
         if isinstance(other, Number) :
             return CSGScaling(self, other)
         else:
             return CSGIntersection(self, other)

     def __sub__(self, other) :
         return CSGDifference(self, other)

  %}
 }

%extend mshr::CSGCGALDomain3D
{
  PyObject* get_vertices()
  {
    npy_intp adims[2] = {static_cast<npy_intp>(self->num_vertices()), 3};
    PyArrayObject* array = reinterpret_cast<PyArrayObject*>(PyArray_EMPTY(2,
                                                                          adims,
                                                                          NPY_DOUBLE,
                                                                          0));
    double* data = static_cast<double*>(PyArray_DATA(array));
    const std::unique_ptr<const std::vector<double>> vertices(std::move(self->get_vertices()));
    std::copy_n(vertices->begin(), vertices->size(), data);

    return reinterpret_cast<PyObject*>(array);
  }

  PyObject* get_facets()
  {
    npy_intp adims[2] = {static_cast<npy_intp>(self->num_facets()), 3};
    PyArrayObject* array = reinterpret_cast<PyArrayObject*>(PyArray_EMPTY(2,         // Num dimensions
                                                                          adims,     // shape
                                                                          NPY_UINTP, // number type
                                                                          0));       // Fortran type storage

    const std::unique_ptr<const std::vector<std::size_t>> facets(std::move(self->get_facets()));
    std::size_t* array_data = static_cast<std::size_t*>(PyArray_DATA(array));
    std::copy_n(facets->begin(), facets->size(), array_data);
    
    return reinterpret_cast<PyObject*>(array);
  }

  // PyObject* convex_hull
}

