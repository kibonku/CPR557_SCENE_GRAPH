====================================================
  Assignment 3 – Scene Graph
  CSE/EE 557 – 3D Graphics Programming
  Name: Kibona
  Email: kibona9@iastate.edu
====================================================

--------
BUILDING
--------
Linux / macOS:
  ./compile-unx.bat

---------
CONTROLS
---------
Camera / Object Mode Toggle:
  [SPACE]   – Toggle between "Move Camera" and "Move Node" modes.
              The current mode is printed to the console.

Scene Graph Navigation:
  [N]       – Advance the scene graph iterator to the next node (DFS order).
              The selected node is marked with '*' in the printout and
              highlighted with a white glow in the viewport.
  [P]       – Print the current scene graph structure to the console.

Camera Controls (only active in "Move Camera" mode):
  [W / S]   – Move camera forward / backward
  [A / D]   – Strafe camera left / right
  [E / Q]   – Move camera up / down
  [↑ ↓ ← →] – Look up / down / left / right

Node Movement (only active in "Move Node" mode):
  [← →]    – Translate current node along X axis (left / right)
  [↑ ↓]    – Translate current node along Y axis (up / down)

Other:
  [C]       – Toggle between perspective and orthographic projection
  [ESC]     – Quit

---------
SCENE GRAPH LAYOUT
---------
Root
    SolarSystem                (Group 1 – positioned left)
        Sun                   (large yellow octahedron)
        Mercury               (small gray cube, offset from Sun)
    Architecture               (Group 2 – positioned right)
        Pyramid               (red-orange pyramid)
        Obelisk               (tall white thin cube)
    Spacecraft                 (Group 3 – positioned top, Phase 5)
        Rocket                (cyan pyramid)
        Satellite             (magenta flat diamond)

The iterator visits nodes in DFS pre-order:
  Root → SolarSystem → Sun → Mercury → Architecture → Pyramid
       → Obelisk → Spacecraft → Rocket → Satellite → (wraps to Root)

---------
PHASE 5 CREATIVITY FEATURES
---------
1. THIRD GROUP NODE ("Spacecraft")
   The scene graph contains three group nodes instead of the required two,
   with two additional leaf objects (Rocket, Satellite) for a total of six
   rendered objects. This demonstrates parent-child transform inheritance:
   moving the "Spacecraft" group translates both Rocket and Satellite together.

2. SELECTION HIGHLIGHT (vertex shader)
   The vertex shader blends a bright-white tint (30 %) into the vertex color
   of any currently-selected node (or all descendants of a selected group).
   This provides immediate visual confirmation of which node the iterator
   points to without requiring any extra UI.

3. SELECTION SPIN ANIMATION
   While in "Move Node" mode, the selected node continuously rotates around
   its local Y-axis. This makes the current node immediately obvious in the
   viewport and also demonstrates that group-node transforms cascade to
   children: selecting a group causes the whole subtree to spin together.

====================================================
