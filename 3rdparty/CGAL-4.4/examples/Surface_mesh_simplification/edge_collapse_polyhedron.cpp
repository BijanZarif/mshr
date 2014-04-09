#include <iostream>
#include <fstream>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

// Adaptor for Polyhedron_3
#include <CGAL/Surface_mesh_simplification/HalfedgeGraph_Polyhedron_3.h>

// Simplification function
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>

// Stop-condition policy
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef CGAL::Polyhedron_3<Kernel> Surface_mesh; 

namespace SMS = CGAL::Surface_mesh_simplification ;

int main( int argc, char** argv ) 
{
  Surface_mesh surface_mesh;
  
  std::ifstream is(argv[1]) ; is >> surface_mesh ;

  // This is a stop predicate (defines when the algorithm terminates).
  // In this example, the simplification stops when the number of undirected edges
  // left in the surface mesh drops below the specified number (1000)
  SMS::Count_stop_predicate<Surface_mesh> stop(1000);
     
  // This the actual call to the simplification algorithm.
  // The surface mesh and stop conditions are mandatory arguments.
  // The index maps are needed because the vertices and edges
  // of this surface mesh lack an "id()" field.
  int r = SMS::edge_collapse
            (surface_mesh
            ,stop
            ,CGAL::vertex_index_map(boost::get(CGAL::vertex_external_index,surface_mesh)) 
                  .edge_index_map  (boost::get(CGAL::edge_external_index  ,surface_mesh)) 
            );
  
  std::cout << "\nFinished...\n" << r << " edges removed.\n" 
            << (surface_mesh.size_of_halfedges()/2) << " final edges.\n" ;
        
  std::ofstream os( argc > 2 ? argv[2] : "out.off" ) ; os << surface_mesh ;
  
  return 0 ;      
}

// EOF //