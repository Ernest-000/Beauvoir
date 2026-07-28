# DOCUMENTATION

Haiii :3
Welcome to Beauvoir's documentation !

## Getting Started

### Linking Beauvoir to your project
As said in the README, you can start by coping an [empty demo project](/demo/empty) which is an empty working project.
In the CMakeFile, you can either link Beauvoir's libraries files by using ```find_package(beauvoir REQUIRED)``` or with the ```add_subdirectory(<path to beauvoir dir> <path to beauvoir's build dir>)``` command.

Use ```#include  <bvr/bvr.h>``` to include default Beauvoir headers.

### Creating a new context
The framework will use ```bvr_book_t``` struct to store all of your game's data. You firstly need to initialize a new instance, then define your book's asset buffer and GC sizes. If 0, these will be set to default values
```c
bvr_book_t book;

// create a new book
bvr_create_book(&book);

// in fact, allocating buffer's sizes is optionnal, but you may want bigger buffers for bigger games.
// bvr_create_book_memories(book, asset_size, garbage_size)
bvr_create_book_memories(&book, BVR_BUFFER_SIZE, 0);
```
Next, you need to create your scene. Scenes are called ```bvr_page_t```. There are two ways of create a new scene. Scene are stores in an array in your game context; each scene are stored in slots (by default there is 8 available slots, you can increase it by overwriting BVR_MAX_PAGE_COUNT). 
To enable a scene you have to set ```book.page``` to point to your active scene (dw, there is functions to do that :>). 

 1. For project where one page is enough

You don't need to allocate new scene, just create your scene over the default slot.
```c
// bvr_create_page(page, name)
bvr_create_page(book.page, "empty");
```
 2. For project where one page is NOT enough
 
You need to create a new scene in an available slot (here, the first one is accessible but after creating this page, you'll need to use the second one; creating a scene on top of another will overwrite it!). After creating your scene, you just need to enable it. 
```c
// bvr_create_page(page, name)
bvr_create_page(&book.slots[0].page, "Side A");
// bvr_enable_page(page)
bvr_enable_page(&book.slots[0].page);
```

### Creating a new window
After that, you may want a window to display your game's visuals :D
```c
// bvr_create_window(window, width, height, title, flags)
bvr_create_window(&book.window, 1920, 1080, "Window", BVR_WINDOW_NONE);
```
Window behaviors can be tweaks using these flags:
- BVR_WINDOW_NONE  
- BVR_WINDOW_RESIZABLE *the window can be scaled by the user* 
- BVR_WINDOW_ALWAYS_ON_TOP  *the window will always be on top of other*
- BVR_WINDOW_BORDERLESS  *the window will not draw borders*
- BVR_WINDOW_FULLSCREEN  *the window will start fullscreen*
- BVR_WINDOW_USER_FRAMEBUFFER  *the window will use a custom framebuffer shader*
 You may want to use this flag to overwrite default window's framebuffer shader. To achieve that, put that macro on top of your main file, before includes.
 ```#BVR_WINDOW_FRAMEBUFFER_PATH "<your shader path>"```
- BVR_WINDOW_DEFAULT *the window can  be resized*

### Main loop
Next, you need to create the main loop. Here is a breakdown of what must happen in the main loop.
 1. Ask to prepare a new frame
 2. Check if the window is still alive
 3. Update
 4. Render
 5. Loop while the game is awake
 
 Your main loop should be like:
 ```c
 while (true)
{
	bvr_new_frame(&book);
	if(!bvr_is_awake(&book)){
		break;
	}
	
	if(bvr_key_presssed(BVR_KEY_ESCAPE)){
		bvr_quit(&book);
	}
	
	// update actors here	
	
	bvr_update(&book);
	
	// draw actors here
	
	bvr_render(&book);
}
```

### Freeing your context
Finally, you need to free you context. For that, just call 	```bvr_destroy_book(&book)```. It will free your window and even your actors!
> Full file can be found [here](/demo/empty/main.c)

## [Actors](src/actor.c) 

Actors are you pawn in the engine. They represent an entity that have a defined behavior.

### Empty actors
Empty actors are used to store generic information or transformations *(e.g. store player's spawn point)*.
To create actors you need to :

 1. Have a valid book & page
 2. Allocate a new actor
 3. Create actor
 
 ```c
 // bvr_alloc_actor(page, type)
 bvr_empty_actor_t* actor = (bvr_empty_actor_t*) bvr_alloc_actor(book.page, BVR_EMPTY_ACTOR);
 
 // bvr_create_actor(actor, name, flags, callback)
 bvr_create_actor(&actor->self, "my actor", 0, NULL);
 ```

### Static actors

Static actors are actors that are render to the screen, but they will not be updated. Using these over Dynamic actors can make your game faster because they will not be considered when updating. 
Static actors can hold a 3D mesh, a shader and a texture.
 1. Have a valid book & page
 2. Allocate a new actor
 3. Create actor
 4. Load your 3D mesh
 5. Load your GLSL shader
 6. Load your texture
 7. Link your texture to your shader through uniforms

## [Mesh](src/mesh.c)
Meshes are points that define a 3D object. Beauvoir can load ```GLB``` and ```OBJ``` 3D file formats. Meshes can be split into vertex groups. 
So that Beauvoir knows how to properly send laced data to the shader, you have to explicit vertex lacing attributes. There is multiple attribute options as such:

 - BVR_MESH_ATTRIB_V2 *uses vec2 for position*
 - BVR_MESH_ATTRIB_V3 *uses vec3 for position*
 - BVR_MESH_ATTRIB_V2UV2 *uses vec2 for position, vec2 for uvs*
 - BVR_MESH_ATTRIB_V3UV2 *uses vec3 for position, vec2 for uvs*
 - BVR_MESH_ATTRIB_V3UV2N3 *uses vec3 for position, vec2 for uvs, uses vec3 for normals*
 - BVR_MESH_ATTRIB_SINGLE *uses a single integer*

There are two ways to create meshes: you can load them from a file or from custom points.

### Create meshes from file
to create a new mesh from a file, you can call this function. For the attribute, you may want to use BVR_MESH_ATTRIB_V3UV2N3.
```c
// bvr_create_mesh(mesh, filepath, attributes);
bvr_create_mesh(&actor->mesh, path, BVR_MESH_ATTRIB_V3UV2N3);
```

### Create custom meshes
To create a new mesh from custom points, you will have to create two ```bvr_mesh_buffer_t```, one for you vertices, one for your indices (this is optional, if not use, set indices type to BVR_NULL or 0). You will have to specify each component.
```c
struct  bvr_mesh_buffer_s {
	char*  data; // pointer to a valid buffer
	uint64  count; // number of elements
	uint32  type; // valid BVR type (e.g. BVR_FLOAT_, BVR_INT32...)
}
```

Next, call ```bvr_create_meshv``` with the correct attribute.
```c
// bvr_create_meshv(mesh, vertices, indices, attributes);
bvr_create_meshv(mesh, &vertices_buffer, &element_buffer, BVR_MESH_ATTRIB_V2UV2);
```

## [Shaders](src/shader.c)
A shader is a piece of code that run on the GPU. Beauvoir uses GLSL as its shader language. 
Shaders are made of shader stages. The vertex stage controls vertex positions in the 3D space, fragment stage define pixels colors. Each stage are separated use an ```#ifdef``` preprocessor.

 - #ifdef \_VERTEX_ to define vertex shader stage.
 - #ifdef \_FRAGMENT_ to define fragment shader stage.

```glsl
#version  400
#ifdef _VERTEX_

// get vertex position
layout(location=0) in  vec3 in_position;
// get vertex uvs
layout(location=1) in  vec2 in_uvs;
// get object's transform
uniform  mat4 bvr_transform;

// uniform block object that contains camera's matrices
layout(std140) uniform bvr_camera {
	mat4 bvr_projection;
	mat4 bvr_view;
};

// the vertex package to send to the fragment shader
// vertex.position (vec3)
// vertex.uvs (vec2)
// vertex.normals (vec3)
out V_DATA vertex;

void main() {
	// calculate the vertex positions in 3D space
	gl_Position = bvr_projection * bvr_view * bvr_transform *  vec4(in_position, 1.0);
	
	// set uvs
	vertex.uvs = in_uvs;
}

#endif

#ifdef _FRAGMENT_

// get the vertex package from the vertex stage
in V_DATA vertex;

void main() {
	// set vertex color 
	gl_FragColor = vec4(vertex.uvs, 1.0, 1.0);
}

#endif
```

Next, you need to create a new shader from this code. 


```c
// bvr_create_shader(shader, filepath, flags)
bvr_create_shader(&actor->shader, "shader.glsl", BVR_VERTEX_SHADER  |  BVR_FRAGMENT_SHADER);
```
Shaders must have at least a vertex and fragment stage to work (geometry or tessellation are not handled yet :<). They can handle these flags :

 - BVR_VERTEX_SHADER *the shader will have to process a vertex shader stage*
 - BVR_FRAGMENT_SHADER *the shader will have to process a fragment shader stage*
 - BVR_FRAMEBUFFER_SHADER *this shader will be created to handle a framebuffer (internal use)* 
 - BVR_SHADER_EXT_LIGHT *this shader will support lighting (light extension)*
 - BVR_SHADER_EXT_SHARE_LAYERS *this shader will support image layers and compositing (layering & compositing extension)*
 
 Then, you have to register and set textures and/or uniforms that you use in your shader. Uniforms are defined with a type *(value's type such as BVR_FLOAT, BVR_TEXTURE_2D, BVR_INT32...)*, a tag (use to find the uniform or mark it for a special use case), the count of values (e.g. for arrays) abd a name.
 Here is the tag list :
 
 - BVR_UNIFORM_NONE *no tag*
 - BVR_UNIFORM_PROJECTION *tag it as a projection matrix*
 - BVR_UNIFORM_TRANSFORM *tag it as a transformation matrix*
 - BVR_UNIFORM_LOCAL_TRANSFORM *tag it as a local transformation matrix*
 - BVR_UNIFORM_TEXTURE *tag it as a texture*
 - BVR_UNIFORM_LAYER_INDEX *tag it as a texture layer index*
 - BVR_UNIFORM_LAYER_INFO *tag it as a texture layer information structure*
 - BVR_UNIFORM_COMPOSITE *tag it as a composite object*

```c
// register a new uniform
bvr_shader_register_uniform(shader, type, tag, count, name);
// set uniform value. Data must be a pointer to an available variable.
bvr_shader_set_uniform(shader, name, data);

// register a new texture
bvr_shader_register_texture(shader, type, texture, name);
// set texture value. Texture must be a pointer to an available texture.
bvr_shader_set_texture(shader, name, texture);

// register a new uniform block object (UBO)
bvr_shader_register_block(shader, name, type, count, index);
``` 
## [Textures](src/image.c)

Texture handling is the key feature of Beauvoir. Nativly, the engine car load ```PNG```, ```PSD```, ```TIF``` and ```BMP```. Each texture can support layering (with offset, displacement, masking and layer filters).
To create a new texture, you can import it from file. Depending on your file, it will either create a 2D or 3D texture.
```c
// bvr_create_texture(texture, filepath, filter, wrap);
bvr_create_texture(&actor->texture, "image.bmp", BVR_TEXTURE_FILTER_LINEAR, BVR_TEXTURE_WRAP_CLAMP_TO_EDGE);
``` 

Then you'll have to link it with your shader. You may want to use BVR_TEXTURE_2D (for 2d images) or BVR_TEXTURE_2D_ARRAY / BVR_TEXTURE_3D (for 2d layerd images). 

### Atlas & Tilesets
Creating tilesets or texture atlas is more complex and might change overtime. To create them, you'll first have to create and define a new ```bvr_atlas_desc_t``` struct. You can either use ```tile_height``` and  ```tile_width``` or  ```tile_per_row``` and  ```tile_per_column``` to explicit how the engine will cut out the atlas. Atlas desc also contains padding options. Then call:
```c
bvr_atlas_desc_t  atlas_desc = { 0 };
atlas_desc.tile_height = 16;
atlas_desc.tile_width = 16;

// you can also use tile_per_row or tile_per_column
// atlas_desc.tile_per_row = 5;
// atlas_desc.tile_per_column = 6;

// bvr_create_texture_atlas(atlas, filepath, atlas_description, filter, wrap);
bvr_create_texture_atlas(&atlas, "tileset.bmp", &atlas_desc, BVR_TEXTURE_FILTER_NEAREST, BVR_TEXTURE_WRAP_CLAMP_TO_EDGE);
 ```
 Atlas need to be register by the shader as a BVR_TEXTURE_2D_ARRAY.

### Images
You can also import images without using them as textures. They can be used as bitmap or the handle custom textures types.
```c
// bvr_create_image(image, filepath);
bvr_create_image(&image, "image.png");
```

### Layers and compositing
Layers and compositing are working together. They handle layers blending and filtering. Layers can blend both with themselfs and the 3D word.
First, create your texture as usual.
```c
// bvr_create_texture(texture, filepath, filter, wrap);
bvr_create_texture(&actor->texture, "image.psd", BVR_TEXTURE_FILTER_LINEAR, BVR_TEXTURE_WRAP_CLAMP_TO_EDGE);
``` 
Next, To support layer blending, you'll have to create a new shader with the ```BVR_SHADER_EXT_SHARE_LAYERS``` flag enable. So, create a compositing shader, this shader will be use to render each layers.
```glsl
#version  400

#ifdef _VERTEX_

layout(location=0) in vec3 in_position;
layout(location=1) in vec2 in_uvs;

uniform mat4 bvr_transform;

out V_DATA vertex;

void main() {
	// only use object transform, don't project it in the 3D world
	gl_Position = bvr_transform * vec4(in_position, 1.0);

	// will get the layer world-space position
	vertex.position = vec3(bvr_transform[3].xyz);
	vertex.uvs = in_uvs;
}
#endif

#ifdef _FRAGMENT_
precision  mediump  float;

in V_DATA vertex;

// composite renderer
uniform sampler2D bvr_composite;

// layer sampler
uniform sampler2DArray bvr_texture;

// layer index
uniform int bvr_layer;

void main() {
	L_DATA layer = create_layer(bvr_layer);
	// layer.index (int)
	// layer.blend (int)
	// layer.opacity (float)

	// sample the composite texture
	vec4 composite = texture(bvr_composite, vertex.uvs + vertex.position.xy);
	// sample layer's image
	vec4 t_sample = texture(bvr_texture, vec3(vertex.uvs, layer.index));

	// call calc_blending(composite, sample, layer_index) to filter and blend layers togethers
	// this function is available with the BVR_SHADER_EXT_SHARE_LAYERS extension
	gl_FragColor  = calc_blending(composite, t_sample, layer);
}

#endif
```

Finally, create the shader and the composite inside the engine.
```c
// create the shader, don't forget the BVR_SHADER_EXT_SHARE_LAYERS flag
bvr_create_shader(&actor->shader, "shader.glsl", BVR_VERTEX_SHADER  |  BVR_FRAGMENT_SHADER  |  BVR_SHADER_EXT_SHARE_LAYERS);

// create a new composite. Composites are a special type of renderbuffers.
// bvr_create_composite(composite, image)
bvr_create_composite(&actor->composite, &actor->texture.image);

// register the texture. Use the type BVR_TEXTURE_2D_LAYER.
bvr_shader_register_texture(&actor->shader, BVR_TEXTURE_2D_LAYER, &actor->texture, "bvr_texture");

// register the composite. Use the type BVR_TEXTURE_2D_COMPOSITE.
bvr_shader_register_texture(&actor->shader, BVR_TEXTURE_2D_COMPOSITE, &actor->composite, "bvr_composite");

// register the layer index. Use the type BVR_TEXTURE_2D_LAYER_STRUCT and the tag BVR_UNIFORM_LAYER_INDEX.
bvr_shader_register_uniform(&actor->shader, BVR_TEXTURE_2D_LAYER_STRUCT, BVR_UNIFORM_LAYER_INDEX, 1, "bvr_layer");

```

---
## Macros

|Name |Usage |Description |Default value|
|---------------------|--------------|-----------------------------------------------------------------------------------------------------------|-------------|
|\_FRAGMENT\_ |GLSL |Define Fragment shader’s section |False |
|\_VERTEX\_ |GLSL |Define Vertex shader’s section |False |
|BVR_NO_FLIP |Engine, OpenGL|Disable auto flipping images for OpenGL |False |
|BVR_ASSERT_FORCE_EXIT|Engine |ASSERT will exit the app |True |
|BVR_INCLUDE_GEOMETRY |Engine |Include geomtry generation algorythmes |False |
|BVR_INCLUDE_DEBUG |Engine |Include debugging functions |True |
|BVR_AUTO_SAVE |Engine |Save scene’s content each time the scene is closed |False |
|BVR_NO_FPS_CAP |Engine |Disable FPS capping |False |
|BVR_SCENE_AUTO_HEAP |Engine |Copy actors to scene’s heap |True |
|BVR_NO_NUKLEAR |Engine |Disable Nuklear API |False |
|BVR_NO_FBX |Engine |Disable FBX loading |False |
|BVR_NO_GLTF |Engine |Disable GLB loading |False |
|BVR_NO_OBJ |Engine |Disable OBJ loading |False |
|BVR_NO_PSD |Engine |Disable PSD loading |False |
|BVR_NO_TIF |Engine |Disable TIF loading |False |
|BVR_NO_BMP |Engine |Disable BMP loading |False |
|BVR_NO_PNG |Engine |Disable PNG loading |False |
|BVR_NO_GROWTH |Engine |Disable growing factors on buffers (less memory but it will take more time to add data to lists or strings)|False |

  

## Functions

|Name |Declaration |Usage|
|-------------|------------------------------------------------------------|---------|
|calc_light |vec4 calc_light(vec4 color, V_LIGHT light, V_DATA vertex) |GLSL |
|create_layer |L_DATA create_layer(int layer) |GLSL |
|calc_blending|vec4 calc_blending(vec4 composite, vec4 pixel, L_DATA layer)|GLSL |
