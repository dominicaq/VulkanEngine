# VulkanEngine
Inside this project are multiple abstractions of what Vulkan does under the hood. My main focus was rendering (vke_renderer) and understanding the rendering pipeline. I implemented a multi light fragment shader, frame buffer, and the implementation of real time shadows (shadow mapping). The rest of the code was me following a tutorial to understand Vulkan.

There are no assets supplied in the repository, if you want to add your own assets and run the project you need to edit the template gameobject in vke_application.cpp in the loadGameObjects() function. This project takes .obj files only and does not bind textures.

![demoimage](https://user-images.githubusercontent.com/38144873/204107921-c03d1065-0d96-4def-8a61-ee9516a267bd.png)
