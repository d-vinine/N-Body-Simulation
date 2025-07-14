# N-Body Simulation (Barnes-Hut Algorithm)

This project is a 2D N-body simulation that uses the **Barnes-Hut algorithm** to efficiently compute gravitational interactions between a large number of particles (up to ~100,000) in real time. It uses **Raylib** for rendering and visualization.


## Requirements

- **C compiler** 
- **Make** (optional, for building)

## Barnes-Hut Algorithm (Brief Overview)

Instead of computing forces between every pair of particles (which is O(N²)), Barnes-Hut organizes particles into a quadtree. Each node approximates the mass of all particles within it. When calculating forces, distant nodes are treated as a single body, reducing the number of computations significantly.
Read more about the algorithm on the [Barnes-Hut Wiki](https://en.wikipedia.org/wiki/Barnes%E2%80%93Hut_simulation).

## Notes

- Performance may vary based on system specs and number of particles.
