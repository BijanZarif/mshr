import mshr
import tempfile, os

# A simple unit cube but with some vertices perturbed slightly to test the FuzzyPointMap
cube = """
solid ascii
facet normal 0 0 0
  outer loop
    vertex 1e-12 0 1
    vertex 0 1 0
    vertex 0 -1e-11 0
  endloop
endfacet

facet normal 0 0 0
outer loop
  vertex 0 0 1
  vertex 0 1 1
  vertex 0 1 1e-13
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 0 0 1
  vertex 1 0 1
  vertex 0 1 1
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 1 0 1
  vertex 1 1 1
  vertex 0 1 1
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 1 0 1
  vertex 1 0 0
  vertex 1 1 1
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 1 0 0
  vertex 1 1 0
  vertex 1 1 1
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 1 0 0
  vertex 0 0 0
  vertex 1 1 0
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 0 0 0
  vertex 0 1 0
  vertex 1 1 0
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 1 1 1
  vertex 1 1 0
  vertex 0 1 1
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 1 1 0
  vertex 0 1 0
  vertex 0 1 1
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 0 0 1
  vertex 0 0 0
  vertex 1 0 1
endloop
endfacet
facet normal 0 0 0
outer loop
  vertex 0 0 0
  vertex 1 0 0
  vertex 1 0 1
endloop
endfacet
endsolid
"""

fd, filename = tempfile.mkstemp(suffix=".stl")
os.write(fd, cube)

s = mshr.Surface3D(filename)
d = mshr.CSGCGALDomain3D(s)
os.close(fd)
os.remove(filename)

assert d.num_holes() == 0
