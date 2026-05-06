To run the code it is best to import the folder to Visual Studio and run it there.

First we must make a scene input file which follows Nan's input file https://www.cs.rit.edu/~ncs/Courses/571/LabData/otherInfo.html</br>
There are some changes mainly to use Photon Mapping add a line under line 5 with the letter 'P'</br>

The wall object follows this format:
1st Line: W, the four (4) vertices (x, y, z) of the wall given in counter-clockwise order</br>
2nd Line: ambient color (r, g, b), diffuse color (r, g, b), specular color (r, g, b), Ka, Kd, Ks, Ke, Kr, Kt</br>

Next move the file to the respective directory (most likely ../out/build/x64-debug)

Finally, run the code and it should output a raytraced_scene.ppm file which you can use an online ppm file viewer to view the image.
