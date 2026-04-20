# Voxel Editor

A level editor which prerenders voxel enviroments to images.

The editor is comprised of 6 stages

    **Tiles** Define tiles to make up the geometry of the level.
    
    **Material** Place materials and voxel meshes on said geometry.
    
    **Rope** Place ropes (defined as Bezier Curves) between points.
    
    **Generation** Paint generative effects to be applied to the defined voxel geometry.
    
    **Light** Paint light sprites to represent lit and unlit regions of the level.
    
    **Render** Compute the final image.

The level can then be previewed with a selected palette, the palette is made up of lit colours, in shadow colours and fog/background colour.

![Render Screenshot](/example2.png)
![Render Screenshot](/example3.png)
![Render Screenshot](/Progress%20Photos/Overshadow%20With%20lighting%20test.png)
![Render Screenshot](/Progress%20Photos/Celluar%20Tower.png)
