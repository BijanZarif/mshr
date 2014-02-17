// Copyright (C) 2014 Benjamin Kehlet
//
// This file is part of mshr.
//
// mshr is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// mshr is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with mshr.  If not, see <http://www.gnu.org/licenses/>.


#include <mshr/VTPFileReader.h>
#include <dolfin/log/log.h>

#include <vtkXMLPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>


namespace mshr
{
void VTPFileReader::read(const std::string filename, 
                         std::vector<std::array<double, 3> > vertices,
                         std::vector<std::vector<std::size_t> > facets)
{
  //get all data from the file
  vtkXMLPolyDataReader* reader = vtkXMLPolyDataReader::New();
  reader->SetFileName(filename.c_str());
  reader->Update();
  vtkPolyData* polydata = reader->GetOutput();
 
  //get the number of points the file contains
  const vtkIdType num_points = polydata->GetNumberOfPoints();
  vertices.resize(num_points);

  const vtkIdType num_polys = polydata->GetNumberOfPolys();
  facets.resize(num_polys);

  //read in all of the points
  for(vtkIdType i = 0; i < num_points; i++)
    polydata->GetPoint(i, vertices[i].data());

 
  vtkCellArray* TriangleCells = polydata->GetPolys();
  vtkIdType npts;
  vtkIdType *pts;
  vtkIdType facet_counter = 0;
 
  while(TriangleCells->GetNextCell(npts, pts))
  {
    dolfin_assert(npts == 3);
    for (int i = 0; i < 3; i++)
      (facets[facet_counter])[i] = pts[i];

    facet_counter++;
  }

  dolfin_assert(facet_counter == num_polys);
}

}
