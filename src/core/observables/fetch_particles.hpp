/*
 * Copyright (C) 2010-2022 The ESPResSo project
 *
 * This file is part of ESPResSo.
 *
 * ESPResSo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ESPResSo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FETCH_PARTICLES_HPP
#define FETCH_PARTICLES_HPP

#include "grid.hpp"
#include "cells.hpp"

#include <utils/Span.hpp>

#include <algorithm>
#include <cstddef>
#include <vector>
#include <set>
#include <functional>

/** Fetch a group of particles.
 *
 *  @param ids particle identifiers
 *  @return array of particle copies, with positions in the current box.
 */
inline std::vector<std::reference_wrapper<Particle>> fetch_particles(std::vector<int> const &ids) {
  auto const ids_set = std::set<int>{ids.begin(), ids.end()};
  auto const local_particles = ::cell_structure.local_particles();
  std::vector<std::reference_wrapper<Particle>> local_particle_refs;
  std::copy_if(local_particles.begin(), local_particles.end(),
	       std::back_inserter(local_particle_refs),
	       [&ids_set](Particle &p) { return ids_set.count(p.id()) != 0; });
  return local_particle_refs;
}
#endif
