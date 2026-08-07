# VBD-cs488-final-project
 
Navigation
- [Description](#description)
- [User Guide](#user-guide)
- [Compilation](#compilation)
- [Implementation](#implementation)
- [Summary of Objectives](#summary-of-objectives)
- [References](#references)

## Description


## User Guide

## Compilation

(OUTDATED. FOLLOW THE SAME BUILD STEPS FROM A0)
### Windows

#### Build Steps
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release 
```
#### Output
The executable will be located at:
```
build\Release\CS488.exe
```
---
#### Build Environment
- Windows version: 25H2 (OS Build 26200.8457)
- cmake version: 4.2.1
- Building for: Visual Studio 17 2022
- Windows SDK version: 10.0.26100.0
- C compiler identification: MSVC 19.44.35222.0
- CXX compiler identification: MSVC 19.44.35222.0

### Linux
This project has not been compiled on Linux.

### Mac
This project has not been compiled on Mac.


## Implementation
<!-- Algorithms, data structures, and complexities,
Platform and system dependence or independence, global constants and configurability,
Input/output syntax, pre- and post-processing,
Data and code sources, the re-use and adaptation of existing code, and any acknowledgment of external sources.
Caveats, bugs, cautions, assumptions. -->
### Algorithms, data structures, and complexities

### global constants and configurability,

### Input/output syntax, pre- and post-processing,

### Acknowledgments

I use the same linear algebra and stb headers that were used in the assignments. Code to calculate the view and projection matrices are also from the assignment starter code. 

The neo hookean model I implemented is adapted from [l2]. It is the same but ommits the logarithmic term, exactly as was done in the VBD paper [l1]. In order to derive the analytical form of the force and energy hessian, I follow the method used in course notes by one of co-authors of the stable neo hookean model paper [l3]. I also used several identities provided in these notes. 

I referenced code from one of the official implementations provided by the authors of the VBD paper [c1]. I did not use any code from them, but I did cross reference my implementation of the analytical Neo-Hookean model against their's. This did help me catch an error in the derivations of the textbook that I referenced [l3]. I also use material parameters from their paper and implementation [l1, c1].  

Thank you to this git hub with obj files for various platonic solids.
https://github.com/cjhoward/platonic-solids/blob/master/

Also thank you to this math stack exchange for their 5-tet cube decomposition.
https://math.stackexchange.com/questions/5145804/partitioning-a-cube-into-five-tetrahedra


## Summary of Objectives

#### Objective 1: Basic Energy Formulation

<!-- Define and solve the inertial energy terms for a set of points. Use the inertia and acceleration as an initial guess. -->

This objective is done.

#### Objective 2: Boundaries

<!-- Add a spring energy term to keep points above the world plane. Points should be pushed into the half space that lies in the direction of the plane’s normal. Generalize this so that the boundary can be defined by a set of planes. -->

This objective is done.

#### Objective 3: Simple Softbody Models

<!-- Define a single tetrahedron model and elastic material parameters. Add a material component to the VBD solver to keep the tetrahedron in its original shape. Define the elastic energy using a stable Neo-Hookean model and use finite differences to approximate the force and stiffness. -->

This objective is done.

#### Objective 4: Damping

<!-- To oppose large velocities, add a Rayleigh damping force proportional to the stiffness.  -->

This objective is done. (not sure if i want it to apply to collision but is okay either way)


#### Objective 5: Collision Detection and Resolution

<!-- Define support functions for triangles and tetrahedra. Use GJK to find collisions between two triangle/tetrahedral meshes. Add a spring energy term to the solver based on the penetration distance.  -->



#### Objective 6: Analytic Force and Energy-Hessian

<!-- Compute the elastic force and energy hessian terms analytically. -->

This objective is done. and i did an extra little material damping term for funsies i.g.

#### Objective 7: Full Convergence

<!-- Use the correct threshhold to allow the simulation to fully converge. Store per frame data and compare these results to the VBD solver to determine an adequate number of iterations for convergence. -->


#### Objective 8: Modeling and Stability Benchmarks

<!-- Model scenes with several elastic bodies. Create graphs to demonstrate how the stability depends on scene geometry and simulation parameters. Confirm or refute anecdotal evidence about the stability of VBD being highly dependent on simulation parameters, and the optimal parameters being significantly different for different scenes. Draw conclusions about the viability of the algorithm for game physics. If it is not viable in its current form, identify areas of future work that could improve the algorithm. -->


## References 
### Literature References
[l1] Anka He Chen, Ziheng Liu, Yin Yang, and Cem Yuksel. 2024. Vertex Block Descent. ACM Trans. Graph. 43, 4, Article 116 (July 2024), 16 pages. https://doi.org/10.1145/3658179

[l2] Breannan Smith, Fernando De Goes, and Theodore Kim. 2018. Stable Neo-Hookean Flesh Simulation. ACM Trans. Graph. 37, 2, Article 12 (April 2018), 15 pages. https://doi.org/10.1145/3180491

[l3] Theodore Kim and David Eberle. 2020. Dynamic deformables: implementation and production practicalities. In ACM SIGGRAPH 2020 Courses (SIGGRAPH '20). Association for Computing Machinery, New York, NY, USA, Article 23, 1–182. https://doi.org/10.1145/3388769.3407490

[l4] T. F. Gast and C. Schroeder. 2015. Optimization integrator for large time steps. In Proceedings of the ACM SIGGRAPH/Eurographics Symposium on Computer Animation (SCA '14). Eurographics Association, Goslar, DEU, 31–40.

### Code References
[c1] https://github.com/AnkaChan/Gaia/tree/main


