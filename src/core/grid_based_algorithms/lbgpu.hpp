/*
   Copyright (C) 2010-2018 The ESPResSo project

   This file is part of ESPResSo.

   ESPResSo is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ESPResSo is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/** \file
 *  %Lattice Boltzmann implementation on GPUs.
 *
 *  Implementation in lbgpu.cpp.
 */

#ifndef LB_GPU_H
#define LB_GPU_H

#include "config.hpp"

#ifdef LB_GPU
#include "utils.hpp"

/* For the D3Q19 model most functions have a separate implementation
 * where the coefficients and the velocity vectors are hardcoded
 * explicitly. This saves a lot of multiplications with 1's and 0's
 * thus making the code more efficient. */
#define LBQ 19

/** Note these are used for binary logic so should be powers of 2 */
#define LB_COUPLE_NULL 1
#define LB_COUPLE_TWO_POINT 2
#define LB_COUPLE_THREE_POINT 4

/** \name Parameter fields for lattice Boltzmann
 *  The numbers are referenced in \ref mpi_bcast_lb_params
 *  to determine what actions have to take place upon change
 *  of the respective parameter. */
/*@{*/
#define LBPAR_DENSITY 0   /**< fluid density */
#define LBPAR_VISCOSITY 1 /**< fluid kinematic viscosity */
#define LBPAR_AGRID 2     /**< grid constant for fluid lattice */
#define LBPAR_TAU 3       /**< time step for fluid propagation */
#define LBPAR_FRICTION                                                         \
  4 /**< friction coefficient for viscous coupling between particles and fluid \
     */
#define LBPAR_EXTFORCE 5 /**< external force acting on the fluid */
#define LBPAR_BULKVISC 6 /**< fluid bulk viscosity */
#define LBPAR_BOUNDARY 7 /**< boundary parameters */
#ifdef SHANCHEN
#define LBPAR_COUPLING 8
#define LBPAR_MOBILITY 9
#endif
/*@}*/

#if defined(LB_DOUBLE_PREC) || defined(EK_DOUBLE_PREC)
typedef double lbForceFloat;
#else
typedef float lbForceFloat;
#endif

/**-------------------------------------------------------------------------*/
/** Parameters for the lattice Boltzmann system for GPU. */
typedef struct {
  /** number density (LB units) */
  float rho[LB_COMPONENTS];
  /** mu (LJ units) */
  float mu[LB_COMPONENTS];
  /** viscosity (LJ) units */
  float viscosity[LB_COMPONENTS];
  /** relaxation rate of shear modes */
  float gamma_shear[LB_COMPONENTS];
  /** relaxation rate of bulk modes */
  float gamma_bulk[LB_COMPONENTS];
  /**      */
  float gamma_odd[LB_COMPONENTS];
  float gamma_even[LB_COMPONENTS];
  /** flag determining whether gamma_shear, gamma_odd, and gamma_even are
   *  calculated from gamma_shear in such a way to yield a TRT LB with minimized
   *  slip at bounce-back boundaries
   */
  bool is_TRT;
  /** friction coefficient for viscous coupling (LJ units)
   *  Note that the friction coefficient is quite high and may
   *  lead to numerical artifacts with low order integrators
   */
  float friction[LB_COMPONENTS];
  /** amplitude of the fluctuations in the viscous coupling
   *  Switch indicating what type of coupling is used, can either
   *  use nearest neighbors or next nearest neighbors.
   */
  int lb_couple_switch;

  float lb_coupl_pref[LB_COMPONENTS];
  float lb_coupl_pref2[LB_COMPONENTS];
  float bulk_viscosity[LB_COMPONENTS];

  /** lattice spacing (LJ units) */
  float agrid;

  /** time step for fluid propagation (LJ units)
   *  Note: Has to be larger than MD time step!
   */
  float tau;

  /** MD timestep */
  float time_step;

  unsigned int dim_x;
  unsigned int dim_y;
  unsigned int dim_z;

  unsigned int number_of_nodes;
  unsigned int number_of_particles;
#ifdef LB_BOUNDARIES_GPU
  unsigned int number_of_boundnodes;
#endif
  /** Flag indicating whether fluctuations are present. */
  int fluct;
  /** to calculate and print out physical values */
  int calc_val;

  int external_force_density;

  float ext_force_density[3 * LB_COMPONENTS];

  unsigned int your_seed;

  unsigned int reinit;

#ifdef SHANCHEN
  /** mobility. They are actually LB_COMPONENTS-1 in number, we leave
   *  LB_COMPONENTS here for practical reasons
   */
  float gamma_mobility[LB_COMPONENTS];
  float mobility[LB_COMPONENTS];
  float coupling[LB_COMPONENTS * LB_COMPONENTS];
  int remove_momentum;
#endif // SHANCHEN

} LB_parameters_gpu;

/** Conserved quantities for the lattice Boltzmann system. */
typedef struct {

  /** density of the node */
  float rho[LB_COMPONENTS];
  /** velocity of the node */

  float v[3];

} LB_rho_v_gpu;
/* this structure is almost duplicated for memory efficiency. When the stress
   tensor element are needed at every timestep, this features should be
   explicitly switched on */
typedef struct {
  /** density of the node */
  float rho[LB_COMPONENTS];
  /** velocity of the node */
  float v[3];
  /** pressure tensor */
  float pi[6];
} LB_rho_v_pi_gpu;

typedef struct {

  lbForceFloat *force_density;
  float *scforce_density;
#if defined(VIRTUAL_SITES_INERTIALESS_TRACERS) || defined(EK_DEBUG)

  // We need the node forces for the velocity interpolation at the virtual
  // particles' position However, LBM wants to reset them immediately after the
  // LBM update This variable keeps a backup
  lbForceFloat *force_density_buf;
#endif

} LB_node_force_density_gpu;

typedef struct {

  float force_density[3];

  unsigned int index;

} LB_extern_nodeforcedensity_gpu;

/************************************************************/
/** \name Exported Variables */
/************************************************************/
/*@{*/

/** Switch indicating momentum exchange between particles and fluid */
extern LB_parameters_gpu lbpar_gpu;
extern LB_rho_v_pi_gpu *host_values;
extern LB_extern_nodeforcedensity_gpu *extern_node_force_densities_gpu;
#ifdef ELECTROKINETICS
extern LB_node_force_density_gpu node_f;
extern bool ek_initialized;
#endif

/*@}*/

/************************************************************/
/** \name Exported Functions */
/************************************************************/
/*@{*/

void lb_GPU_sanity_checks();

void lb_get_device_values_pointer(LB_rho_v_gpu **pointeradress);
void lb_get_boundary_force_pointer(float **pointeradress);
void lb_get_lbpar_pointer(LB_parameters_gpu **pointeradress);
void lb_get_para_pointer(LB_parameters_gpu **pointeradress);
void lattice_boltzmann_update_gpu();

/** (Pre-)initialize data structures. */
void lb_pre_init_gpu();

/** Perform a full initialization of the lattice Boltzmann system.
 *  All derived parameters and the fluid are reset to their default values.
 */
void lb_init_gpu();

/** (Re-)initialize the derived parameters for the lattice Boltzmann system.
 *  The current state of the fluid is unchanged.
 */
void lb_reinit_parameters_gpu();

/** (Re-)initialize the fluid. */
void lb_reinit_fluid_gpu();

/** Reset the forces on the fluid nodes */
void reset_LB_force_densities_GPU(bool buffer = true);

/** (Re-)initialize the particle array */
void lb_realloc_particles_gpu();
void lb_realloc_particles_GPU_leftovers(LB_parameters_gpu *lbpar_gpu);

void lb_init_GPU(LB_parameters_gpu *lbpar_gpu);
void lb_integrate_GPU();
#ifdef SHANCHEN
void lb_calc_shanchen_GPU();
void lattice_boltzmann_calc_shanchen_gpu();
#endif
void lb_free_GPU();
void lb_get_values_GPU(LB_rho_v_pi_gpu *host_values);
void lb_print_node_GPU(int single_nodeindex,
                       LB_rho_v_pi_gpu *host_print_values);
#ifdef LB_BOUNDARIES_GPU
void lb_init_boundaries_GPU(int n_lb_boundaries, int number_of_boundnodes,
                            int *host_boundary_node_list,
                            int *host_boundary_index_list,
                            float *lb_bounday_velocity);
#endif
void lb_init_extern_nodeforcedensities_GPU(
    int n_extern_node_force_densities,
    LB_extern_nodeforcedensity_gpu *host_extern_node_force_densities,
    LB_parameters_gpu *lbpar_gpu);

void lb_calc_particle_lattice_ia_gpu(bool couple_virtual);

void lb_calc_fluid_mass_GPU(double *mass);
void lb_calc_fluid_momentum_GPU(double *host_mom);
void lb_remove_fluid_momentum_GPU(void);
void lb_calc_fluid_temperature_GPU(double *host_temp);
void lb_get_boundary_flag_GPU(int single_nodeindex, unsigned int *host_flag);
void lb_get_boundary_flags_GPU(unsigned int *host_bound_array);

void lb_set_node_velocity_GPU(int single_nodeindex, float *host_velocity);
void lb_set_node_rho_GPU(int single_nodeindex, float *host_rho);

void reinit_parameters_GPU(LB_parameters_gpu *lbpar_gpu);
void lb_reinit_extern_nodeforce_GPU(LB_parameters_gpu *lbpar_gpu);
void lb_reinit_GPU(LB_parameters_gpu *lbpar_gpu);
int lb_lbnode_set_extforce_density_GPU(int ind[3], double f[3]);
void lb_gpu_get_boundary_forces(double *forces);
void lb_save_checkpoint_GPU(float *host_checkpoint_vd,
                            unsigned int *host_checkpoint_boundary,
                            lbForceFloat *host_checkpoint_force);
void lb_load_checkpoint_GPU(float *host_checkpoint_vd,
                            unsigned int *host_checkpoint_boundary,
                            lbForceFloat *host_checkpoint_force);

void lb_lbfluid_remove_total_momentum();
void lb_lbfluid_fluid_add_momentum(float momentum[3]);
void lb_lbfluid_calc_linear_momentum(float momentum[3], int include_particles,
                                     int include_lbfluid);
void lb_lbfluid_particles_add_momentum(float velocity[3]);

void lb_lbfluid_set_population(const Vector3i &, float[LBQ], int);
void lb_lbfluid_get_population(const Vector3i &, float[LBQ], int);

void lb_lbfluid_get_interpolated_velocity_at_positions(double const *positions,
                                                       double *velocities,
                                                       int length);
uint64_t lb_fluid_rng_state_gpu();
void lb_fluid_set_rng_state_gpu(uint64_t counter);
uint64_t lb_coupling_rng_state_gpu();
void lb_coupling_set_rng_state_gpu(uint64_t counter);
/*@{*/

#endif /* LB_GPU */

#endif /* LB_GPU_H */
