// Copyright (C) 2016 Benjamin Kehlet
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

#ifndef FUZZY_POINT_SET_H
#define FUZZY_POINT_SET_H

#include <utility> // defines less operator for std::pair
#include <map>
#include <set>
#include <algorithm>

// TODO: Template over dimension.
//template <std::size_t dim>
class FuzzyPointMap
{
 public:
  FuzzyPointMap(double tolerance)
    : tolerance(tolerance)
  {}
  //-----------------------------------------------------------------------------
  template <typename Point>
  std::size_t insert_point(const Point& p)
  //-----------------------------------------------------------------------------
  {
    const std::size_t index = get_index(p);
    if (index == points.size())
    {
      // insert the points
      points.push_back(std::array<double, 3>{p[0], p[1], p[2]});
      maps[0].insert(std::make_pair(p[0], index));
      maps[1].insert(std::make_pair(p[1], index));
      maps[2].insert(std::make_pair(p[2], index));
    }

    return index;
  }
  //-----------------------------------------------------------------------------
  template <typename Point>
  std::size_t forced_insert_point(const Point& p)
  //-----------------------------------------------------------------------------
  {
    const std::size_t index = points.size();

    points.push_back(std::array<double, 3>{p[0], p[1], p[2]});
    maps[0].insert(std::make_pair(p[0], index));
    maps[1].insert(std::make_pair(p[1], index));
    maps[2].insert(std::make_pair(p[2], index));

    return index;
  }
  //-----------------------------------------------------------------------------
  template<typename Point>
  bool contains(const Point& p)
  //-----------------------------------------------------------------------------
  {
    const std::size_t index = get_index(p);
    return index < points.size();
  }
  //-----------------------------------------------------------------------------
  const std::array<double, 3>& operator[](std::size_t i) const
  //-----------------------------------------------------------------------------
  {
    return points[i];
  }
  //-----------------------------------------------------------------------------
  const std::vector<std::array<double, 3>>& get_points() const
  //-----------------------------------------------------------------------------
  {
    return points;
  }
  //-----------------------------------------------------------------------------
  std::size_t size() const
  //-----------------------------------------------------------------------------
  {
    return points.size();
  }

private:
  template<typename Point>
  std::size_t get_index(const Point& p)
  {
    typedef std::multimap<double, std::size_t>::iterator iterator;

    // Check if a nearby point exists
    std::array<std::set<std::size_t>, 3> results;

    for (std::size_t i = 0; i < 3; i++)
    {
      iterator lb = maps[i].lower_bound(p[i]-tolerance);
      iterator ub = maps[i].upper_bound(p[i]+tolerance);

      // Note that we shouldn't check if ub != maps[i].end(), so if. the last
      // entry in maps[i] one is less than p[i]+tolerance), it will also return
      // end()
      if (lb != maps[i].end())
      {
        iterator it = lb;
        while (it != ub)
        {
          results[i].insert(it->second);
          it++;
        }
      }
    }

    // This is a naive implementation. Just iterate the first set repeatedly and
    // remove elements not present in the other sets.
    for (std::size_t i = 1; i < 3; i++)
    {
      for (std::set<std::size_t>::iterator it = results[0].begin(); it != results[0].end(); it++)
      {
        if (results[i].find(*it) == results[i].end())
          results[0].erase(it);
      }
    }

    // Found (at least one) close point. Return index.
    if (results[0].size() > 0)
      return *results[0].begin();
    else
      // No close point found. Insert new
      return points.size();
  }

  const double tolerance;
  std::array<std::multimap<double, std::size_t>, 3> maps;
  std::vector<std::array<double, 3>> points;
};

#endif
