#version 150

in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D texFramebuffer;
uniform float time = 0;
uniform float beat = 0;




float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(float x, float y){
	return rand(vec2(x, y));
}

void main() {
    vec4 col = vec4(texture(texFramebuffer, Texcoord));
    float m = distance(vec2(0.5,0.5), Texcoord);
    m -= 0.25;
    m *= 0.01;
    m = max(0.0001, m);
    vec2 offset = vec2( rand(Texcoord.x, time)-0.5f, rand(Texcoord.y, time)-0.5f );
    offset *= 0.25;
    offset.x -= mod(offset.x,1);
    offset.y -= mod(offset.y,1);
    offset /= 0.25;
    vec4 col2 = vec4(texture(texFramebuffer, Texcoord + m * offset));
    float r = max(col.r - col.g, col2.r - col2.g);

    if(r > 0){
    	col -= fract(col2/r);
    }else{
    	col -= fract(col2/2);
    }

    outColor = length(col.rgb) >= length(vec3(0.5)) ? vec4(1) : vec4(0,0,0,0.5);
    if(r > 0){
	    outColor.g *= col.g;
	    outColor.b *= col.b;
    }

    outColor.b += 1 + beat * (m*10000);
}
