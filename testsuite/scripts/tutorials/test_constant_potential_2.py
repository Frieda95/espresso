# Copyright (C) 2019-2022 The ESPResSo project
#
# This file is part of ESPResSo.
#
# ESPResSo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ESPResSo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import unittest as ut
import importlib_wrapper
import numpy as np

tutorial, skipIfMissingFeatures = importlib_wrapper.configure_and_import(
    "@TUTORIALS_DIR@/constant_potential/constant_potential_part2.py")


@skipIfMissingFeatures
class Tutorial(ut.TestCase):

    def test_potential_difference(self):
        # Test that the applied potential difference equals the one from
        # integrating Poisson equation
        self.assertAlmostEqual(
            tutorial.POTENTIAL_DIFF/tutorial.measured_potential_difference, 1, 1)

if __name__ == "__main__":
    ut.main()
