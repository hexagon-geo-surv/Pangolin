@start vertex
#version 150 core

in vec2 a_position;
in vec2 a_texcoord;

uniform mat4 u_T_cm;
uniform vec2 u_scale;

out vec2 v_uv;

void main() {
    gl_Position = u_T_cm * vec4(a_position, 0.0, 1.0);
    v_uv = a_texcoord;
}

@start fragment
#version 150 core
#include "utils.glsl.h"
#include "sdf.glsl.h"

uniform sampler2D u_font_atlas;

in vec2  v_uv;
out vec4 FragColor;

uniform int  u_font_bitmap_type;
uniform float u_scale;
uniform vec2  u_max_sdf_dist_uv;
uniform vec3  u_color;

float SdfScaleFactor(vec2 tex_uv, vec2 unit_range) {
    vec2 screenTexSize = vec2(1.0)/fwidth(tex_uv);
    return max(0.5*dot(unit_range, screenTexSize), 1.0);
}

void main() {
    vec3 texel = texture(u_font_atlas, v_uv).xyz;
    float opacity;

    if(u_font_bitmap_type == 0) {
        opacity = texel.r;
    }else{
        float sd = (u_font_bitmap_type == 1) ? texel.r: median(texel.r, texel.g, texel.b);
        float sdf = SdfScaleFactor(v_uv, u_max_sdf_dist_uv)*(sd - 0.5);
        opacity = clamp( sdf + 0.5, 0.0, 1.0);
    }

//    FragColor = vec4( u_color, opacity );
    FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );

}
