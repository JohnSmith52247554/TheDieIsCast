uniform sampler2D texture;
uniform float time;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;

    vec4 color = texture2D(texture, uv);

    if (color.a == 0.0)
    {
        gl_FragColor = color;
        return;
    }

    vec3 rgb = color.rgb;

    float temp = 0.5 * (sin(5.0 *  time) + 1);

    float scanLineFactor = 0.5 * (sin(uv.y * 3.0 + time * 5.0) + 1);

    float factor = 0.8 * scanLineFactor;

    rgb = factor * vec3(42.0 /255.0, 111.0 /255.0, 151.0 /255.0) + (1 - factor) * rgb;
    rgb *= 0.2  * temp + 1;
    
    gl_FragColor = vec4(rgb, 1.0);
}