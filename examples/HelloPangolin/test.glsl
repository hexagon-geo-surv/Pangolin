@start vertex
#version 150 core

in vec4 a_position;
out vec4 pos;

void main() {
    pos = a_position;
}

@start geometry
#version 150 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 pos[];
out vec2 v_pos;

void main() {
    vec2 corners[4] = vec2[](
        vec2(-1.0,-1.0), vec2(+1.0,-1.0),
        vec2(-1.0,+1.0), vec2(+1.0,+1.0)
    );

    for(uint i=0u; i < 4u;  ++i) {
        v_pos = corners[i];
        gl_Position = vec4(v_pos, 0.0, 1.0);
        EmitVertex();
    }

    EndPrimitive();}

@start fragment
#version 150 core

uniform sampler2D u_font_atlas;
uniform sampler2D u_font_offsets;

in vec2 v_pos;
out vec4 FragColor;

void main() {
    vec2 uv = (v_pos + vec2(1.0)) / 2.0;
    vec3 texel1 = texture(u_font_atlas, uv).xyz;
    vec3 texel2 = texture(u_font_offsets, uv).xyz;
    vec4 texel3 = texelFetch(u_font_offsets, ivec2(0,0), 0);

    FragColor = vec4(texel3.xyz,1.0);
//    FragColor = vec4(1.0,0.0,0.0,1.0);
}
