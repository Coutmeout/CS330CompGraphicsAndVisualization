# CS-330 Computer Graphics and Visualization — 3D Kitchen Scene

A 3D scene built with OpenGL, GLFW, and GLM depicting a kitchen 
countertop with a rolling pin, cutting board, chef's knife, and cup. 
The scene features textures, Phong lighting, and full camera navigation 
via keyboard and mouse.

---

## Software Design

### New Design Skills

Working on this project has helped me develop my skills and understanding of 3D compositional design. Moreover, it taught me how to take a real-world object and break it down into geometric primitives. For example, a rolling pin is not a singular shape, but a composition of a large cylinder (rollin part), two smaller cylinders (for the handles), and two spheres (for the pin handles end caps). Each shape was individually scaled, positioned, and textured. Being able to analyze an object and identify which primitive shapes would best represent it is a skill I will continue using in 3D modeling, visualization, etc.

### Design Process

I followed an iterative, object-first approach when designing my project. First, I sketched out each object in the scene, giving me a clear breakdown of what shapes would be needed depending on my choices. It also helped me choose which texture or material would best suit the object. I added each object one at a time and used the camera navigation to meticulously inspect each object from multiple angles. This approach helped avoid compounding errors and make it simple to isolate and amend positioning issues as they occurred.

### Application to Future Work

Again, the decomposition skill is crucial to my profession and goes beyond graphics programming. The same skills can be applied when designing software architecture, UI layouts, data models, etc., allowing a complex problem to be broken down into smaller, more manageable steps. The iterative, milestone-based system used in this course translate directly to Agile development practices.

---

## Program Development

### New Development Strategies

My favorite strategy used in this project was function-based scene management. Instead of writing all the rendering logic inline, the scene is organized into separate functions: LoadSceneTextures() handles texture loading, DefineObjectMaterials() centralizes material definitions, and SetTransformations encapsulates all matrix math. This separation of concerns ensures that adding a new object to the scene was as simple as setting a handful of values and calling the appropriate draw method. This is in contrast to needing to understand or repeat underlying OpenGL calls.

### The Role of Iteration

Iteration was a key aspect of my development process. From camera position, object scale, and texture UV tiling to light position and material shininess values, each evolves through repeated adjustments and visual inspections. Each milestone built on the previous, which made verifying each layer an independent process before adding complexity.

### Evolution of approach

Early on, changes were made to values, and it became tedious. Over the course of the assignments and milestone, my approach evolved toward utilizing named constants, reusable helper functions, and descriptive comments. Once we reached the final project, adding new objects became accomplishable in minutes vs hours. Having the infrastructure for transformations, materials, and textures was undoubtedly a huge tool in my development process.

---

## Computer Science and My Goals

### Educational Pathway

This course introduced concepts like linear algebra, matrix transformations, etc., that appeared in other courses. Being able to tie these concepts to actual work was eye opening and taught me a lot. For example, understanding how the model, view, and projection matrices function simultaneously to transform a 3D point to a 2D screen pixel provides a solid foundation in computer vision, machine learning, and advanced rendering. 

### Professional Pathway

The skills I've developed in this course are applicable to my professional roles in data visualization, simulation, game development, and UI/UX engineering. Learning and understanding the rendering pipeline, texture mapping, lighting models, and camera systems gave me a strong foundation for future work with an engine like Unity, or WebGL-based visualization libraries. This course taught me to think spatially and manage complex states across a rendering loop. Additionally, I learned how to debug visual outputs systematically, and is a skill I will always cherish.
