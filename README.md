# VBD-cs488-final-project
 
Navigation
- [Description](#description)
- [User Guide](#user-guide)
- [Compilation](#compilation)
- [Implementation](#implementation)
- [Summary of Objectives](#summary-of-objectives)
- [References](#references)

## Description

For my project, I built a soft-body physics simulation using an energy optimization method called Vertex Block Descent. My simulation includes 3 types of objects. I've implemented soft bodies built from tetrahedral models with deformation energy defined by a stable neo-hookean model. Ridgid spherical particles are included to better demonstrate the behavior of the algorithm with the presence of stiff interactions. Finally, there is a static boundary plane. Collisions between the plane and either of the two body types are modeled using a stiff spring energy term. I've used my implementation to demonstrate some of the failure conditions of the algorithm and draw some conclusions about its viability as a physics engine for games. 

The algorithm fails to converge to a good solution when there are very high mass to stiff ratios. Hard constraints (like the boundary plane) are also difficult to model correctly since the optimal stiffness (visually speaking) depends on the mass and momentum of the object (as well as the material stiffness of the soft bodies). The rayleigh damping factor has a similar problem, where the best damping factor is observed to be dependent on the material parameters of the soft bodies. (See the implementation section for details about the convergence criterions). This aligns well with anecdotal evidence from other developers who have implemented the algorithm.

In all of my tests, the algorithm converged in a similar number of steps. In all my tests, the algorithm stops yielding any significant changes after less than 10 iterations. In well-conditioned cases, 2-5 steps are already enough for good convergence. But in the failure cases I demonstrated, even running the algorithm for 100 iterations yields no visible improvement.

Since a game scene can change at runtime, a game physics engine must be relatively insensitive to the scene configuration. With VBD, it is not good enough to find a single "good" parameterization, as this "good" solution might change while the game progresses. Furthermore, the characteristics of a "good" parameterization are not simple enough for most non-technical developers to understand easily. This would, of course, make it hard for developers to want to migrate to this system. 

The formulation does have some merits. The integrator itself is relatively easy to implement and very flexible in how you can add new force terms and other elements to the simulation. The formulation is also highly parallel in nature and is robust to large timesteps. Simple scenes yielded good results for timesteps of 1/6s to over 1s (a timestep of 1/60s is used in all my demo scenes). Neither of these were fully explored here, but both are important to note. That said, the numerical issues need to be improved significantly before it can be properly adopted into a game engine.   

For future work, it would be interesting to see how much of an improvement the AVBD algorithm can make on the stiffness issues for hard constraints (this algorithm uses lagrange multipliers to model them). It would also be interesting to see if the collision stiffness can be tuned on a per-contact basis rather than providing a single value for everything. 

## User Guide
 * Press W to move towards the scene POI, S to move away from it.
 * Press A to rotate left around the scene POI and D to rotate right.
 * Press Q to move up, Z to move down.
 * Press P to pause and unpause the simulation
 * Press K to enter or exit the debug scene (this was only really used to test configuration as I built them. It is also useful if you which to tune some of the parameters in the debugScene function and see what changes).
 * When not in  the debug scene, Press left(right) arrow to move to the next(previous) demo world.
 * Press R to restart the current world.

Shaders and model files can be found in the resources folder. All code is in src.

## Screenshots

screenshot1 has 2 tetrahedrons, one with high material stiffness and one with a low material stiffness. We can see the less stiff one undergoing some deformations after hitting the ground and the stiff one holding its shape well.

screenshot2 has a 5-tet cube model and some rigid particles resting stably on the ground after making contact.

You can see these scene being animated in my demo video.

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

The main complexities come from the math and not the algorithms. The Neo-Hookean model I use 

### Global Constants and Configurability

The debugScene function shows how to configure the simulation world, there are a lot of interesting pa

### Input/output syntax
Command line args
```
./CS488.exe <resource path> -c <convergence> -i <max iterations>
```

All of the arguments are optional and have sensible default values.
 - \<resource path\>: the relative path to the resources folder. defaults to "./resources".
 - \<convergence\>: if this is 0, the program will not compute the convergence criterions. If it is 1 it will compute and print them to the console.
 - \<max iterations\>: the maximum number of iteration to use in the VBD optimizer.

Convergence criterions are as follow:
 * grad loss: the normed energy gradient after an iteration. The goal of VBD is to minimize this quantity.  
 * relative grad loss: grad loss from the current iteration over grad loss from the first iteration. The relative amount of grad loss that remains to be optimized. 
 * abs change grad loss: The difference between grad loss in successive iterations.
 * rel change grad loss: (current grad loss - previous grad loss) / previous grad loss. The relative improvement between iterations.
 *  normed dx: The amount by which the iteration changed the vertex positions.
 *  abs change dx: How much the above quantity changed between iterations.
 *  rel change dx: Same as the above but relative to the norm of dx in the previous iteration.

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

This objective is done. The inertial energy contribution is determined using the formulas provided in the VBD paper.

Code for this can be found in the ```World``` class in main.cpp

#### Objective 2: Boundaries
This objective is done. Correction forces are determined using the error energy formula provided in the VBD paper.

Code for this can be found in the ```World``` class in main.cpp

#### Objective 3: Simple Softbody Models

This objective is done. The energy model is the same stable Neo-hookean model used in the VBD paper. Force and hessian are approximated using finite differencing. 

Code for this can be found in the ```NeoHookenModel``` namespace in main.cpp

#### Objective 4: Damping

This objective is done. A rayleigh damping force is proportional to the energy hessian is added according to the formula in the VBD paper. 

Code for this can be found in the ```World``` class in main.cpp

#### Objective 5: Collision Detection and Resolution

This objective is not complete.

#### Objective 6: Analytic Force and Energy-Hessian

This objective is done. I use an FEM approach to solve for the force and hessian analytically. The method is given in detail in [l3].

Code for this can be found in the ```NeoHookenModel``` namespace in main.cpp

#### Objective 7: Full Convergence

This objective is done. I compute various stopping criteria that describe the convergence of the system. The most important ones are the energy gradient, as this reaches 0 we are approaching a solution to the dynamics equations of the system. I also compute metrics about the change is dx (which each iteration is solving for).

The data for this can be streamed into a file for processing and analysis.

You may wish to run the simulation and pause around interesting key frames to inspect the data qualitatively.

Code for this can be found in the ```World``` class in main.cpp

#### Objective 8: Modeling and Stability Benchmarks

This objective is complete. I demonstrated several scenes with some well conditioned cases and several different failure cases. (See project description for more analysis).

In my proposal, I stated that I would create some graphs of this data. However, I believe that this is not really a good way to represent my findings. Since my goal with this algorithm is to implement it into a game engine context, the visual results are far more important than the numeric ones. Several simple scenes have seemly high loss criterions (even when considered with respect to the number of vertices) but have good visual results. The number themselves also fail to capture how/why the algorithm fails which is also important. 

I believe that any line/scatter plot diagrams I could make would need to cut out a lot of the data to be visually parsable (for example cherry picking certain frames) and this did not feel like a good way to represent my work and learning. I've opted instead to give more qualitative analysis (while referencing the simulation parameters and value ranges when appropriate) as I think these observations offer more insight into the algorithm. I've included the scenes that I used in my demo as a visual representation of that data.

## References 
### Literature References
[l1] Anka He Chen, Ziheng Liu, Yin Yang, and Cem Yuksel. 2024. Vertex Block Descent. ACM Trans. Graph. 43, 4, Article 116 (July 2024), 16 pages. https://doi.org/10.1145/3658179

[l2] Breannan Smith, Fernando De Goes, and Theodore Kim. 2018. Stable Neo-Hookean Flesh Simulation. ACM Trans. Graph. 37, 2, Article 12 (April 2018), 15 pages. https://doi.org/10.1145/3180491

[l3] Theodore Kim and David Eberle. 2020. Dynamic deformables: implementation and production practicalities. In ACM SIGGRAPH 2020 Courses (SIGGRAPH '20). Association for Computing Machinery, New York, NY, USA, Article 23, 1–182. https://doi.org/10.1145/3388769.3407490

[l4] T. F. Gast and C. Schroeder. 2015. Optimization integrator for large time steps. In Proceedings of the ACM SIGGRAPH/Eurographics Symposium on Computer Animation (SCA '14). Eurographics Association, Goslar, DEU, 31–40.

### Code References
[c1] https://github.com/AnkaChan/Gaia/tree/main


