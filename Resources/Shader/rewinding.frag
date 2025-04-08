uniform sampler2D texture;
uniform float time;
uniform float fadeFactor;    // 褪色强度（0.0-1.0）
uniform float rgbIntensity;   // RGB分离强度（0.0-0.1）
uniform vec2 rgbDirection;    // 分离方向标准化向量（如(1.0,0.5)）
uniform float distortionFactor; // 边缘畸变强度（0.0-1.0）
uniform bool useShader; 

// 随机噪声函数
float rand(vec2 seed) 
{
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

// 边缘畸变函数
vec2 distortUV(vec2 uv, float factor)
{
    vec2 center = vec2(0.5);
    vec2 dist = uv - center;
    float r = length(dist);
    float theta = atan(dist.y, dist.x);
    
    // 非线性边缘弯曲
    float distortion = 1.0 + factor * (r * r);
    
    // 计算新的UV坐标
    uv = center + vec2(cos(theta), sin(theta)) * distortion * r;
    
    return uv;
}

// 动态RGB分离计算
vec3 rgbSplit(vec2 uv)
{   
    float directionFloat = 0.3;
    vec2 directionOffset = vec2(
        sin(time * 7.0) * directionFloat, // 调整速度和范围
        cos(time * 5.0) * directionFloat
    );

    // 计算新的方向向量
    vec2 dynamicDir = rgbDirection + directionOffset;
    
    // 生成动态偏移量（随时间变化）
    vec2 offset = dynamicDir * rgbIntensity * 
        (sin(time * 10.0) * 0.5 + 0.5); // 波动效果

    // 各通道差异采样
    float r = texture2D(texture, uv - offset * 1.2).r;
    float g = texture2D(texture, uv).g;
    float b = texture2D(texture, uv + offset).b;

    // 对各通道应用不同的噪声强度
    float rNoise = rand(uv * 10.0 + time) * 0.02;
    float bNoise = rand(uv * 8.0 - time) * 0.02;
    r = texture2D(texture, uv - offset * 1.2 + vec2(rNoise)).r;
    b = texture2D(texture, uv + offset + vec2(bNoise)).b;
    
    return vec3(r, g, b);
}

void main() 
{
    if (useShader == false)
    {
        gl_FragColor = texture2D(texture, gl_TexCoord[0].xy);
        return;
    }

    vec2 uv = gl_TexCoord[0].xy;

    // 获取当前像素的完整颜色，包括alpha分量
    vec4 sampledColor = texture2D(texture, uv);
    
    if (sampledColor.a == 0.0)
    {
        gl_FragColor = sampledColor;
        return;
    }

    // 应用边缘畸变
    uv = distortUV(uv, distortionFactor);

    // 应用RGB分离
    vec3 color = rgbSplit(uv);
    
    // 扫描线效果
    float scanLine = sin(uv.y * 80.0 + time * 8.0) * 0.06;
    color += scanLine;
    
    // 褪色处理
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(color, vec3(luminance), fadeFactor);
    
    // CRT色彩模拟
    color *= vec3(1.0, 0.95, 0.9); // 暖色滤镜
    color = pow(color, vec3(1.1)); // Gamma增强

    // 计算像素到屏幕边缘的距离
    float distanceFromEdge = 1 - min(uv.x, min(uv.y, min(1.0 - uv.x, 1.0 - uv.y)));

    // 使用 smoothstep 函数创建平滑过渡
    float edgeFadeFactor = smoothstep(0.95, 1.0, distanceFromEdge);
    color *= (1 - edgeFadeFactor); // 使边缘颜色变暗

    gl_FragColor = vec4(color, 1.0);
}