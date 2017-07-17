#version 400

uniform sampler2D sampler0;



uniform vec3 baseColor = vec3(0.8f, 0.3f, 0.2f);

uniform vec3 La = vec3(1.0);	    	//Ambient light intensity
uniform	vec3 Ld = vec3(0.6);		    //Diffuse light intensity
uniform	vec3 Ls = vec3(0.6);		    //Specular light intensit
uniform vec3 Ra = vec3(0.5);			//Ambient reflectivity
uniform	vec3 Rd = vec3(0.5);			//Diffuse reflectivity
uniform	vec3 Rs = vec3(0.6);			//Specular reflectivity
uniform	float Shininess = 0.8;	        //Specular shininess factor


in vec3 norm_eye;
in vec3 light_eye;
in vec3 view_eye;
    
    
in vec2 vTexCod;
out vec4 outColor;

void main()
{
   // int value = sampler0;
    if(vTexCod != vec2(0.0f,0.0f))
        outColor = texture2D(sampler0, vTexCod);
        
    else {
        vec3 ambient, diffuse, spec;
        vec3 n =  normalize( norm_eye );
        vec3 s = normalize( light_eye);
        vec3 v = -normalize( view_eye );
        vec3 r = reflect( -s, n );

        ambient = La * Ra;
        float sDotN = max( dot( s, n ), 0.0 );

        diffuse = Ld * Rd * sDotN;
        spec = Ls * Rs * pow( max( dot(r,v) , 0.0 ), Shininess );
        outColor = vec4( vec3(ambient + diffuse), 1 ) * vec4(baseColor, 1) + vec4( spec, 1 );
    }
}
