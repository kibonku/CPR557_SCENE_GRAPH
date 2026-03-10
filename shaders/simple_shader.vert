#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Pushdata
{
    mat4 transform;
    vec3 push_color;
} pushdata;

void main()
{
    gl_Position = pushdata.transform * vec4(position, 1.0);

    // Phase 5 highlight: when push_color is non-zero (i.e. the node is selected),
    // blend 30% of push_color into the vertex color to create a bright glow effect.
    // When push_color is (0,0,0) the output is just the regular vertex color.
    float highlightStrength = length(pushdata.push_color) > 0.01 ? 0.30 : 0.0;
    fragColor = mix(color, pushdata.push_color, highlightStrength);
}
