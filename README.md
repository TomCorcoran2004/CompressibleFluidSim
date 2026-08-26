# 2D Compressible CFD Solver

A high-performance **2D compressible-flow CFD solver written from scratch in modern C++**.

The project solves the inviscid compressible Euler equations using a finite-volume formulation. The main focus is not only implementing the numerical methods, but exploring how CFD solvers can be structured for high performance through **data-oriented design, SIMD-friendly memory layouts, multithreading, and eventually GPU acceleration**.

The solver is currently under active development.

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

---

## Performance

Performance is a major design goal of the project.

The solver uses a **structure-of-arrays / structure-of-vectors layout** rather than storing complete fluid states as individual structs.

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
- [ ] AMR
- [ ] General unstructured meshes
- [ ] Mesh import
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

---

## Dependencies

The project is currently developed on Windows using **Visual Studio 2022** and modern C++.
It is designed to not rely on any external dependencies, to enable maximum portability. 
The core numerical solver is written directly in C++ and does not rely on an external CFD framework.

---
