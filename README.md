
## 🏄🌒 mozu

OpenGL ray tracer based on the "Ray Tracing in One Weekend series".

### 📷Progress:

#### Sky color & sphere hit

![framework screenshot](mozu/screens/1.png "First trial")
> Normalizing the y coordinate and interpolating between two colors. Checking if the rays hit the sphere and displaying the color.

#### Adding the ground sphere

![framework screenshot](mozu/screens/2.png "Ground sphere & coloring both using the normals")

#### Computing rays for a matte material - Lambertian
![framework screenshot](mozu/screens/3-unit-sph.png "Lambertian scattering")
> Computed the resulting color by picking a random point inside a unit radius sphere, whose normal starts from the initial ray hit point. This is done recursively until a ray doesn't hit anything or a maximum depth is reached.

![framework screenshot](mozu/screens/3-unit-vec.png "Lambertian scattering w/ unit vector method")
> True Lambertian distribution, achieved by picking random points in the sphere and then normalizing them.
> 
> Shadows are less pronounced and spheres are lighter.

![framework screenshot](mozu/screens/3-20samples.png "Lambertian scattering w/ unit vector method and 20 samples")
> True Lambertian with 20 samples per pixel.
#### Computing rays for a metallic material 
![framework screenshot](mozu/screens/4.png "Scattering for two metallic materials with roughness 0.8 & 0.3")
> For metallic surfaces,rays aren't randomly scattered. Luckily, GLSL has the reflect function which returns the reflected ray. Randomizing the reflected ray by choosing a random endpoint as we did for the matte surface, we can get more rough (fuzzy) surfaces.

#### Computing rays for a glass material - Dielectric
![framework screenshot](mozu/screens/5.png "Refraction on a dielectric surface")
>For a dielectric surface, a ray is split into both a reflected one and a refracted one. Assuming that all rays are only refracted we get this result.

![framework screenshot](mozu/screens/6.png "Refraction & reflection on a dielectric surface")
>Using Schlick's polynomial aproximation we can determine if rays must pe reflected instead of refracted, giving the final result.

#### Cornell Box
![framework screenshot](mozu/screens/7.png "Cornell box")
>Added axis-aligned rectangles to make the Cornell box.

#### Emissive Material

![framework screenshot](mozu/screens/8.png "Emissive Materials")
>To make things more realistic, I added an emissive material for some ceiling lights & spheres. Rays hitting an emissive surface won't scatter so they retain their true albedo color output.

#### Scene Layout
![framework screenshot](mozu/screens/9.png "Scene Layout")
>👌
### 📚Resources:

[Ray Tracing in One Weekend](https://raytracing.github.io/)
