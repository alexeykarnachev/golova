in vec3 fragPosition;

uniform vec2 u_screen_size;
uniform float u_time;

out vec4 finalColor;

float hash31(vec3 p){ 
    return fract(sin(dot(p,vec3(10.9898,70.233,40.6474)))*45758.5433);
}

float starnoise(vec3 rd){ //rd needs to be normalized
    const vec3 amps = vec3(.6,.45,.15); //sum>1 for scaling 
    const vec4 cs = vec4(3./5.,4./5.,5./13.,12./13.); //cos and sines using paythagorean triplets
    const mat3 arbitrartyRotAndScale = mat3(dot(amps,vec3(1,cs.x,cs.z)),amps.z*cs.w,amps.y*cs.y,
                                        -cs.w*amps.z,dot(amps,vec3(1.,1.,cs.z)), 0,
                                        -cs.y*amps.y,0,dot(amps,vec3(1,cs.x,1.)));
                                         
    float c = 0.;
    vec3 p = rd*300.;
	for (float i=0.;i<4.;i++)
    {
        vec3 q = fract(p)-.5;
        vec3 id = floor(p);
        float c2 = smoothstep(.25,0.,dot(q,q));
        c2 *= step(hash31(id),.02-i*i*0.002);
        c += c2;
        p = p*arbitrartyRotAndScale;
    }
    c*=c;
    
    //giroid-based intensity variation
    float g = dot(sin(rd*12.12),cos(rd.yzx*4.512));
    float d=smoothstep(-3.14159265359,-.9,g)*.5+.5*smoothstep(-.3,1.,g);
    c*=d*d*.5+d*.5;
    return c*c;
}

void main(void) {
    vec2 uv = fragPosition.xy / u_screen_size;

    float zoom = 2.-cos(u_time*.2);
    vec2 m = acos(-1.)*mix(vec2(-1,-.5),vec2(1,.5), 0.9);
    
    float cx = cos(m.y),sx=sin(m.y);
    mat3 rx = mat3(1,0,0,0,cx,sx,0,-sx,cx);
        
    float cy = cos(m.x),sy=sin(m.x);
    mat3 ry = mat3(cy,0,sy,0,1,0,-sy,0,cy);
     
    vec3 rd = normalize(vec3(uv-.5, zoom))*rx*ry;
    
    float l = starnoise(rd);

    finalColor = vec4(sqrt(l));
}
