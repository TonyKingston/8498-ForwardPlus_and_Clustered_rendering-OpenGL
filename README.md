# README
## Intro
This is my OpenGL rendering project from university, though I've some what taken an interest in trying to clean it up a bit lately.
There are a couple of optimisations and techniques that I've been wanting to implement for a while, and I figured that just using this as a base wouldn't hurt too much.
I might get round to developing it further (or not...)

#### Controls:

WASD and mouse to move the camera. Shift and space raise and lower the camera respectively.
Press 1 to add more lights to the scene.
Pressing 9 will enter debug mode. (forward+ and clustered) It will show the number of lights occupying a tile/cluster
- will halve the number of lights added when pressing 1.
+ will double the number of lights added when pressing 1.

Upon running the application, you will be presented with the choice of four rendering techniques:
forward, deferred, forward+ and clustered.
With forward and clustered, you can also specify to run a depth prepass or not.
If you select forward+, the console will ask if you want to run it using AABBs for light culling.

Feel free to mess around to see each technique in action and assess their performance.

I repurposed the codebase from previous coursework, so there is a lot of superfluous code that I haven't
had the time to get rid of. The main areas of interest are GameTechRenderer and the various shaders located
in the Shader directory.
