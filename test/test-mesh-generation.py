import mshr
from dolfin import *

# issue 37
g = mshr.Rectangle(Point(0.0, 0.0), Point(2.2, .41)) - mshr.Circle(Point(.2, .2), .05, 40)
m = mshr.generate_mesh(g, 50)

