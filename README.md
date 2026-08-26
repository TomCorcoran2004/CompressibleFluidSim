# 2D Compressible CFD Solver

A high-performance **2D compressible-flow CFD solver written from scratch in modern C++**.

The project solves the inviscid compressible Euler equations using a finite-volume formulation. The main focus is not only implementing the numerical methods, but exploring how CFD solvers can be structured for high performance through **data-oriented design, SIMD-friendly memory layouts, multithreading, and eventually GPU acceleration**.

The solver is currently under active development.

---

## Overview

The current solver is built around a face-based finite-volume formulation for the **2D compressible Euler equations**:

\[
\frac{\partial \mathbf{U}}{\partial t}
+
\nabla \cdot \mathbf{F}(\mathbf{U})
= 0
\]

with conserved state

\[
\mathbf{U}
=
\begin{bmatrix}
\rho \\
\rho u \\
\rho v \\
E
\end{bmatrix}
\]

and an ideal-gas equation of state

\[
p = (\gamma - 1)
\left(
E - \frac{1}{2}\rho(u^2+v^2)
\right)
\]

where:

- \(\rho\) — density
- \(u,v\) — velocity components
- \(E\) — total energy density
- \(p\) — pressure
- \(\gamma\) — ratio of specific heats

The current implementation uses a **Rusanov / local Lax-Friedrichs numerical flux**, with more accurate approximate Riemann solvers planned.

---

## Current Features

### Solver

- 2D compressible Euler equations
- Cell-centred finite-volume discretisation
- Explicit time integration
- CFL-controlled timestep
- Rusanov numerical flux
- Ideal-gas equation of state
- Face-based flux evaluation
- Internal and boundary face regions
- Slip-wall boundary conditions
- Infrastructure for inflow/outflow boundary conditions
- Structured 2D meshes

### Validation

- Sod shock-tube test
- Exact 1D Euler Riemann solver for comparison
- Newton-Raphson solution of the star-region pressure
- Shock and rarefaction sampling

An isentropic vortex test is planned to provide a more useful multidimensional accuracy/convergence test.

### Visualisation

The project includes real-time visualisation using:

- OpenGL
- GLFW
- GLAD
- ImGui
- ImPlot
- GLM

This allows the state of the simulation to be inspected while the solver is running.

---

## Performance

Performance is a major design goal of the project.

The solver uses a **structure-of-arrays / structure-of-vectors layout** rather than storing complete fluid states as individual structs.

For example, conserved variables are stored conceptually as:

```text
rho   = [ ... ]
rho_u = [ ... ]
rho_v = [ ... ]
E     = [ ... ]
```

rather than:

```text
Cell {
    rho
    rho_u
    rho_v
    E
}
```

This makes sequential processing of individual fields significantly more cache- and SIMD-friendly.

The project also contains a custom aligned `SoV` container used for solver state storage.

### Current CPU Benchmark

Recent Rusanov flux microbenchmarks have reached approximately:

> **~70 million face evaluations / second on a single Ryzen 5 3600 core**

This is a kernel-level performance measurement rather than complete simulation throughput, so it should not be directly compared with end-to-end performance figures from production CFD packages.

Current optimisation work includes:

- contiguous field storage
- aligned allocation
- reducing temporary state construction
- eliminating unnecessary branches
- reducing repeated mesh lookups
- exposing loops to compiler auto-vectorisation
- analysing failed vectorisation
- SIMD-oriented loop restructuring

Manual SIMD intrinsics and wider parallel execution are intended as later optimisation stages.

---

## Data Layout

Solver variables are separated into conserved and derived quantities.

### Conserved State

```cpp
enum class ConservedFields
{
    Rho,
    Rhou,
    Rhov,
    E
};
```

Corresponding to

\[
(\rho,\rho u,\rho v,E)
\]

### Derived State

```cpp
enum class DerivedFields
{
    InvRho,
    u,
    v,
    p,
    c
};
```

where

\[
c = \sqrt{\frac{\gamma p}{\rho}}
\]

is the local speed of sound.

Derived quantities are calculated in dedicated loops to keep the main flux kernels simple and to improve opportunities for vectorisation.

---

## Mesh Representation

The mesh is represented primarily through contiguous face and cell arrays.

Faces contain or reference information such as:

```text
LeftCell[]
RightCell[]
NormalX[]
NormalY[]
InvDl[]
```

This layout allows a flux kernel to process large batches of faces without repeatedly traversing complex mesh objects.

A typical internal-face update follows the form

\[
U_L^{n+1}
=
U_L^n
-
\frac{\Delta t}{\Delta l}F^*
\]

\[
U_R^{n+1}
=
U_R^n
+
\frac{\Delta t}{\Delta l}F^*
\]

where \(F^*\) is the numerical flux through the face.

---

## Rusanov Flux

The current numerical flux is the Rusanov flux:

\[
F^*
=
\frac{1}{2}
\left(
F(U_L)+F(U_R)
\right)
-
\frac{1}{2}
s_{\max}
(U_R-U_L)
\]

with

\[
s_{\max}
=
\max
\left(
|u_{n,L}|+c_L,
|u_{n,R}|+c_R
\right)
\]

where \(u_n\) is velocity normal to the face.

Rusanov is relatively diffusive, but its simplicity makes it useful while developing and profiling the underlying solver architecture.

A less diffusive **HLLC solver** is one of the next major numerical additions.

---

## Project Architecture

The project is progressively being split into distinct systems:

```text
Mesh
 ├── cells
 ├── faces
 ├── geometry
 └── boundary regions

Fluid Solver
 ├── conserved states
 ├── derived states
 ├── flux calculation
 ├── boundary conditions
 └── time integration

Numerics
 ├── equation of state
 ├── Rusanov solver
 ├── exact Riemann solver
 └── future HLLC solver

Rendering
 ├── OpenGL
 ├── ImGui
 └── ImPlot
```

One of the goals is to keep numerical algorithms independent enough from storage and execution backends that alternative implementations can be introduced without redesigning the entire solver.

---

## Design Goals

This project is being developed as both a CFD solver and a performance-engineering project.

The main goals are:

1. **Correctness**

   Establish reliable validation cases before increasing numerical complexity.

2. **Data-oriented architecture**

   Keep hot solver data contiguous and minimise pointer chasing and unnecessary abstraction inside numerical kernels.

3. **CPU performance**

   Explore cache behaviour, compiler optimisation, auto-vectorisation, explicit SIMD and multicore execution.

4. **GPU acceleration**

   Introduce a CUDA execution path for computationally expensive solver kernels.

5. **More general meshes**

   Move beyond the current structured mesh implementation toward unstructured finite-volume meshes.

6. **Better numerical methods**

   Add higher-quality Riemann solvers and eventually higher-order spatial reconstruction.

---

## Roadmap

### Numerics

- [x] 2D Euler equations
- [x] Ideal-gas EOS
- [x] Rusanov flux
- [x] CFL timestep calculation
- [x] Exact Euler Riemann solver
- [x] Sod shock-tube validation
- [x] Slip-wall boundary condition
- [ ] HLLC flux
- [ ] Isentropic vortex validation
- [ ] MUSCL reconstruction
- [ ] Slope limiters
- [ ] Second-order time integration
- [ ] Additional compressible-flow validation cases

### Mesh

- [x] Structured 2D mesh
- [x] Face-based topology
- [x] Boundary regions
- [ ] General unstructured meshes
- [ ] Mesh import
- [ ] More complex geometry
- [ ] Improved mesh preprocessing

### Performance

- [x] Structure-of-arrays state layout
- [x] Aligned solver storage
- [x] SIMD-oriented data layout
- [x] Compiler vectorisation analysis
- [ ] Manual SIMD kernels
- [ ] Multicore CPU execution
- [ ] Improved thread scaling
- [ ] CUDA backend
- [ ] GPU timestep reduction
- [ ] CPU/GPU performance comparison
- [ ] Large-mesh benchmarking

### Application

- [x] Real-time OpenGL rendering
- [x] ImGui integration
- [x] ImPlot integration
- [ ] Improved field visualisation
- [ ] Runtime solver configuration
- [ ] Mesh loading UI
- [ ] Simulation statistics/profiling
- [ ] Result export
- [ ] More complete standalone CFD application

---

## Dependencies

The project is currently developed on Windows using **Visual Studio 2022** and modern C++.

Current external dependencies include:

- GLFW
- GLAD
- OpenGL 3.3+
- GLM
- Dear ImGui
- ImPlot
- stb

The core numerical solver is written directly in C++ and does not rely on an external CFD framework.

---

## Development Environment

Current development hardware:

```text
CPU: AMD Ryzen 5 3600
GPU: NVIDIA GeForce RTX 2060
OS:  Windows
IDE: Visual Studio 2022
```

The RTX 2060 is intended to be used for the initial CUDA implementation.

---

## Why This Project?

A major motivation for the project is understanding what happens below the level of a typical CFD library.

Rather than treating the numerical solver as a black box, the project is intended to explore the entire path from

```text
Euler equations
      ↓
finite-volume discretisation
      ↓
Riemann solver
      ↓
mesh/data representation
      ↓
CPU execution
      ↓
SIMD
      ↓
multicore execution
      ↓
GPU execution
```

This makes the project as much about **high-performance scientific computing** as CFD itself.

---

## Status

This is an experimental project under active development.

The solver is not intended to compete with mature production packages such as OpenFOAM, SU2 or commercial CFD software. Features such as viscous flow, turbulence modelling, robust general-purpose meshing and higher-order schemes are currently outside the implemented feature set.

The immediate focus is building a small, well-understood solver with a strong numerical foundation and then progressively improving its performance and capability.

---

## Future Direction

The longer-term aim is to turn the solver into a more complete CFD application supporting:

```text
Unstructured meshes
        +
Higher-order numerics
        +
Multicore CPU execution
        +
SIMD
        +
CUDA
        +
Interactive visualisation
```

while retaining a relatively small and understandable codebase.

The project is intentionally being developed incrementally so that numerical changes and performance changes can be measured independently.
