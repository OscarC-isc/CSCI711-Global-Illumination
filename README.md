The 1st checkpoint, setting the scene, set the scene to resemble the raytracing scene from Whitted, 1980.
Godot was used to render the scene and values are stored in the "CSCI711 Raytracing Checkpoint 1" PDF file.

The 2nd checkpoint, Raytracing Framework, using the values from checkpoint 1, 
Trace rays though the camera to produce an image
Non-recursive ray tracing
Visible surface determination
If no intersection, use background color.

![checkpoint2](Checkpoint2/CSCI711_checkpoint2.jpg)


The 3rd checkpoint, Basic Shading, Implemented Phong Illumination Model for Shading<br/>
![checkpoint3](Checkpoint3/CSCI711_checkpoint3.jpg)

The 4th checkpoint, procedural texture shading to create a checkerboard floor<br/>
![checkpoint4](Checkpoint4/CSCI711_checkpoint4.jpg)

The 5th checkpoint, Recursive raytracing, Implemented reflections<br/>
![checkpoint5](Checkpoint5/CSCI711_checkpoint5.jpg)

The 6th checkpoint, Implementing refractions<br/>
1.0 index of refraction<br/>
![checkpoint6_1.0refraction](Checkpoint6/CSCI711_checkpoint6_1.0refraction.jpg)

1.33 index of refraction<br/>
![checkpoint6_1.33refraction](Checkpoint6/CSCI711_checkpoint6_1.33refraction.jpg)

1.33 index of refraction and 0.1Kt for reflection<br/>
![checkpoint6_1.33refraction](Checkpoint6/CSCI711_checkpoint6_1.33refraction0.1Kt.jpg)

1.5 index of refraction<br/>
![checkpoint6_1.5refraction](Checkpoint6/CSCI711_checkpoint6_1.5refraction.jpg)

1.5 index of refraction and 0.1Kt for reflection<br/>
![checkpoint6_1.5refraction](Checkpoint6/CSCI711_checkpoint6_1.5refraction0.1Kt.jpg)

The 7th checkpoint, Implementing Tone Reproduction with Reinhard and Ward Models.<br/>
To change the Illuminace Range a lightscale constant (0.5 for Low, 2.5 for Med, 5.0 for High) was multipled to each light color.

Ward Model Lo-Range lighting (0.5 Scaling)<br/>
![checkpoint7 WardLo](Checkpoint6/CSCI711_checkpoint7_WardLo.jpg)

Ward Model Mid-Range lighting (2.5 Scaling)<br/>
![checkpoint7 WardMed](Checkpoint6/CSCI711_checkpoint7_WardMed.jpg)

Ward Model Hi-Range lighting (5.0 Scaling)<br/>
![checkpoint7 WardHI](Checkpoint6/CSCI711_checkpoint7_WardHi.jpg)

Reinhard Model Lo-Range lighting (0.5 Scaling)<br/>
![checkpoint7 ReinhardLo](Checkpoint6/CSCI711_checkpoint7_ReinhardLo.jpg)

Reinhard Model Mid-Range lighting (2.5 Scaling)<br/>
![checkpoint7 ReinhardMed](Checkpoint6/CSCI711_checkpoint7_ReinhardMed.jpg)

Reinhard Model Hi-Range lighting (5.0 Scaling)<br/>
![checkpoint7 ReinhardHi](Checkpoint6/CSCI711_checkpoint7_ReinhardHi.jpg)
