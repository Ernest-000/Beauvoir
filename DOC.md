# DOCUMENTATION
Here you'll find some important informations if you want to get learn how to use Beauvoir!
I'll try to reference everything, but this will take time...

## Macros
|Name                 |Usage        |Description                                                                                                |Default value|
|---------------------|--------------|-----------------------------------------------------------------------------------------------------------|-------------|
|\_FRAGMENT\_           |GLSL          |Define Fragment shader’s section                                                                           |False          |
|\_VERTEX\_             |GLSL          |Define Vertex shader’s section                                                                             |False          |
|BVR_NO_FLIP          |Engine, OpenGL|Disable auto flipping images for OpenGL                                                                    |False          |
|BVR_ASSERT_FORCE_EXIT|Engine        |ASSERT will exit the app                                                                                   |True         |
|BVR_INCLUDE_GEOMETRY |Engine        |Include geomtry generation algorythmes                                                                     |False          |
|BVR_INCLUDE_NUKLEAR  |Engine        |Include Nuklear API                                                                                        |True         |
|BVR_INCLUDE_DEBUG    |Engine        |Include debugging functions                                                                                |True         |
|BVR_AUTO_SAVE        |Engine        |Save scene’s content each time the scene is closed                                                         |False          |
|BVR_NO_FPS_CAP       |Engine        |Disable FPS capping                                                                                        |False          |
|BVR_SCENE_AUTO_HEAP  |Engine        |Copy actors to scene’s heap                                                                                |True         |
|BVR_NO_FBX           |Engine        |Disable FBX loading                                                                                        |False          |
|BVR_NO_GLTF          |Engine        |Disable GLB loading                                                                                        |False          |
|BVR_NO_OBJ           |Engine        |Disable OBJ loading                                                                                        |False          |
|BVR_NO_PSD           |Engine        |Disable PSD loading                                                                                        |False          |
|BVR_NO_TIF           |Engine        |Disable TIF loading                                                                                        |False          |
|BVR_NO_BMP           |Engine        |Disable BMP loading                                                                                        |False          |
|BVR_NO_PNG           |Engine        |Disable PNG loading                                                                                        |False          |
|BVR_NO_GROWTH        |Engine        |Disable growing factors on buffers (less memory but it will take more time to add data to lists or strings)|False          |

## Functions
|Name         |Declaration                                                 |Usage|
|-------------|------------------------------------------------------------|---------|
|calc_light   |vec4 calc_light(vec4 color, V_LIGHT light, V_DATA vertex)   |GLSL     |
|create_layer |L_DATA create_layer(ivec3 layer)                            |GLSL     |
|calc_blending|vec4 calc_blending(vec4 composite, vec4 pixel, L_DATA layer)|GLSL     |