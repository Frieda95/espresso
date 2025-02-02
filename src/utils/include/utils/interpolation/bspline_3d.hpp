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

#pragma once

#include "utils/Vector.hpp"

#include "detail/ll_and_dist.hpp"
#include "utils/math/bspline.hpp"

#include <array>

namespace Utils {
namespace Interpolation {

/**
 * @brief cardinal B-spline interpolation with internal iteration.
 *
 * @tparam order The number of interpolation points in each direction.
 * @tparam Kernel Callable that can be called at every point with the
 *         index of the point and its weight as arguments.
 *
 * @param pos The point at which the interpolation should be run.
 * @param kernel Function to run for every point.
 * @param grid_spacing The distance between the grid points.
 * @param offset Shift of the grid relative to the origin.
 */
template <int order, typename Kernel>
void bspline_3d(Vector3d const &pos, Kernel const &kernel,
                Vector3d const &grid_spacing, Vector3d const &offset) {
  using Utils::bspline;

  /* The coordinates and relative distance of the assignment cube. */
  auto const block = detail::ll_and_dist<order>(pos, grid_spacing, offset);

  /* Precalc weights that are used multiple times. */
  std::array<double, order> w_y{};
  std::array<double, order> w_z{};
  for (int i = 0; i < order; i++) {
    w_y[static_cast<unsigned>(i)] = bspline<order>(i, block.distance[1]);
    w_z[static_cast<unsigned>(i)] = bspline<order>(i, block.distance[2]);
  }

  std::array<int, 3> ind;
  for (int i = 0; i < order; i++) {
    ind[0] = block.corner[0] + i;
    auto const wx = bspline<order>(i, block.distance[0]);
    for (int j = 0; j < order; j++) {
      ind[1] = block.corner[1] + j;
      auto const wxy = wx * w_y[static_cast<unsigned>(j)];
      for (int k = 0; k < order; k++) {
        ind[2] = block.corner[2] + k;
        kernel(ind, wxy * w_z[static_cast<unsigned>(k)]);
      }
    }
  }
}

/**
 * @brief cardinal B-spline weighted sum.
 */
template <int order, typename T, typename Kernel>
T bspline_3d_accumulate(Vector3d const &pos, Kernel const &kernel,
                        Vector3d const &grid_spacing, Vector3d const &offset,
                        T const &init) {
  T value = init;
  bspline_3d<order>(
      pos,
      [&value, &kernel](std::array<int, 3> const &ind, double w) {
        value += w * kernel(ind);
      },
      grid_spacing, offset);

  return value;
}

} // namespace Interpolation
} // namespace Utils
