/*#version 130

uniform sampler2D source;
uniform float time; // Time to control the pulse
out vec4 FragColor;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 color = texture(source, uv);
    
    // Golden color: RGB (1.0, 0.84, 0.0)
    vec3 golden = vec3(1.0, 0.84, 0.0);
    
    // Pulse effect - modulate the intensity based on time
    float pulse = 0.5 + 0.5 * sin(time); // Sin wave for smooth pulsing
    
    // Apply golden effect with pulsing
    FragColor = vec4(golden * color.rgb * pulse, color.a);
}*/

#version 130

uniform float time;
in vec2 TexCoords;
out vec4 color;

void main()
{
    // Example effect: flicker effect based on time
    float alpha = abs(sin(time));
    color = vec4(8.0, 8.0, 8.0, alpha);
}