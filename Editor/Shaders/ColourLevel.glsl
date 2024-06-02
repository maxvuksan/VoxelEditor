uniform sampler2D u_texture;


const vec3 colour = 






void main()
{
    vec4 sampled_pixel = texture2D(u_texture, gl_TexCoord[0].xy);
    vec4 fog_colour = vec4(0.12, 0.3, 0.44, 1.0);

    vec4 solid_colour = vec4(0.1, 0.06, 0.14, 1.0);

    solid_colour *= sampled_pixel.r;
    solid_colour.a = 1.0;

    gl_FragColor = mix(solid_colour, fog_colour, (1.0 - sampled_pixel.a));
};