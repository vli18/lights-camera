#version 330 core

// Task 4: declare a vec3 object-space position variable, using
//         the `layout` and `in` keywords.
layout(location = 0) in vec3 pos_obj;
layout(location = 1) in vec3 normal_obj;

// Task 5: declare `out` variables for the world-space position and normal,
//         to be passed to the fragment shader
out vec3 pos_world;
out vec3 normal_world;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    // Task 8: compute the world-space position and normal, then pass them to
    //         the fragment shader using the variables created in task 5
    pos_world = vec3(model * vec4(pos_obj, 1));
    normal_world = normalize(mat3(transpose(inverse(model))) * normal_obj);

    // Task 9: set gl_Position to the object space position transformed to clip space
    gl_Position = proj * view * model * vec4(pos_obj, 1.0f);
}
