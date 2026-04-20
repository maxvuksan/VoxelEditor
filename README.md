# Voxel Editor

A level editor which prerenders voxel enviroments to images.


## The Pipeline
The editor is comprised of 6 distinct stages:

🧱 Tiles – Define the base tiles that make up the core geometry of the level.

🎨 Material – Place specific materials and detailed voxel meshes onto the established geometry.

🪢 Rope – Create and place ropes between points using Bézier Curves for natural sagging and tension.

🪄 Generation – Paint generative effects that procedurally modify or enhance the defined voxel geometry.

💡 Light – Map out light sprites to define the lit, shadowed, and unlit regions of the environment.

🖼️ Render – Process all data and compute the final rendered image.

![Render Screenshot](/example2.png)
![Render Screenshot](/example3.png)

The level can then be previewed with a selected palette, the palette is made up of lit colours, in shadow colours and fog/background colour.

![Render Screenshot](/Progress%20Photos/Overshadow%20With%20lighting%20test.png)
![Render Screenshot](/Progress%20Photos/Celluar%20Tower.png)
