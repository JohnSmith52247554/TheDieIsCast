uniform sampler2D texture;

uniform float time;

// 伪随机数生成函数
float rand(vec2 co)
{
    return fract(sin(dot(co.xy / 10000, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = gl_TexCoord[0].xy;

    vec4 color = texture2D(texture, uv);
    if (color.a == 0.0)
    {
        gl_FragColor = color;
        return;
    }

    vec3 color_rgb = color.rgb;

    // 生成随机噪声
    float noise = rand(vec2(uv.y, time));

    // 计算噪点强度
    float noiseStrength = 0.3;

    // 将随机噪声应用到颜色上，只减去噪声值以实现黑色噪点效果
    vec3 resultColor = color_rgb - vec3(noiseStrength * noise);

    // 确保颜色值在0到1之间
    resultColor = clamp(resultColor, 0.0, 1.0);

    // 输出最终颜色
    gl_FragColor = vec4(resultColor, color.a);
}