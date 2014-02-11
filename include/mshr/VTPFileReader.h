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
//

#ifndef __VTP_FILE_READER_H
#define __VTP_FILE_READER_H

#include <string>
#include <vector>
#include <array>

namespace mshr
{

class VTPFileReader
{
public:
  static void read(const std::string filename, 
                   std::vector<std::array<double, 3> > vertices,
                   std::vector<std::array<std::size_t, 3> > facets);
};

}
#endif
