#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform mat4 u_LightVP;
uniform mat4 u_CameraVP;
uniform sampler2D u_SceneColorTexture;
uniform sampler2D u_SceneDepthTexture;
uniform sampler2D u_ShadowMap;
uniform vec4 u_LightColor;

void main()
{
	vec4 baseColor = texture(u_SceneColorTexture, v_TexCoord);
	float sceneDepth = texture(u_SceneDepthTexture, v_TexCoord).r;
	vec4 ndc = vec4(v_TexCoord * 2.0 - 1.0, sceneDepth * 2.0 - 1.0, 1.0);
	vec4 worldPos = inverse(u_CameraVP) * ndc;
	worldPos /= worldPos.w;
	vec4 lightClip = u_LightVP * worldPos;
	lightClip /= lightClip.w;
	vec2 lightUV = lightClip.xy * 0.5 + 0.5;
	float lightDepth = lightClip.z * 0.5 + 0.5;
	float bias = 0.005;
	float shadow = 1.0;
	if (lightUV.x >= 0.0 && lightUV.x <= 1.0 && lightUV.y >= 0.0 && lightUV.y <= 1.0 && lightDepth <= 1.0)
	{
		float shadowMapDepth = texture(u_ShadowMap, lightUV).r;
		shadow = (lightDepth > shadowMapDepth + bias) ? 0.35 : 1.0;
	}
	vec3 mixedColor = baseColor.rgb * u_LightColor.rgb * shadow;
	color = vec4(mixedColor, baseColor.a);
}