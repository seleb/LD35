#version 150

in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D texFramebuffer;
uniform float time = 0;




float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(float x, float y){
	return rand(vec2(x, y));
}

void main() {
    vec4 col = vec4(texture(texFramebuffer, Texcoord));
    vec4 col2 = vec4(texture(texFramebuffer, Texcoord + 0.01 * vec2( rand(Texcoord.x, time)-0.5f, rand(Texcoord.y, time)-0.5f )));
    bool r = col.r > col.g || col2.r > col2.g;

    if(r){
    	col -= col2;
    }

    outColor = length(col.rgb) >= length(vec3(0.5)) ? vec4(1) : vec4(0,0,0,0.5);
    if(r){
	    outColor.g *= col.g;
	    outColor.b *= col.b;
    }
}
