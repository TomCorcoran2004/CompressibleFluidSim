# 2D Compressible CFD Solver

A high-performance **2D compressible-flow CFD solver written from scratch in modern C++**.

The project solves the inviscid compressible Euler equations using a finite-volume formulation. The main focus is not only implementing the numerical methods, but exploring how CFD solvers can be structured for high performance through **data-oriented design, SIMD-friendly memory layouts, multithreading, and GPU acceleration**.

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
- 2D vortex in isentropic flow
- 2D Quadrant Sod Shock
  
---

## Performance

Performance is a major design goal of the project.

The solver uses a **structure-of-arrays / structure-of-vectors layout** rather than storing complete fluid states as individual structs.

### Current CPU Benchmark

Recent benchmarks have reached approximately:

> **~25 million face evaluations / second on a single Ryzen 5 3600 core**
> **~70 million face evaluations / second when fully multithreaded**

### Current GPU Benchmark

Recent benchmarks have reached approximately:

> **~160 million face evaluations / second on my NVIDIA RTX 2060**

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
- [x] Isentropic vortex validation
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
- [x] Multicore CPU execution
- [ ] Improved thread scaling
- [x] CUDA backend

---

## Dependencies

The project is currently developed on Windows using **Visual Studio 2022** and modern C++.
The core numerical solver is written directly in C++ and does not rely on an external CFD framework.

---
