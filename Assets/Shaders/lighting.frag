#include "Shared/LightDefinitions.h"

// TODO: Some materials specify their own specular exponent. Should use that instead of universal magic number
float calculateSpecular(float rFactor) {
    return pow(rFactor, 60);
}

// Blinn-Phong based BRDF.
void calculateLighting(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal, float specSample, inout vec3 diffuseOut, inout vec3 specularOut) {
	//vec3 lightVec = light.pos.xyz - IN.worldPos;
	vec3 lightVec = light.pos.xyz - fragPos;
	//float lambert = max(0.0, dot(incident, normal)) * 0.9;

	float distance = length(lightVec);
	float attenuation = 1.0f - clamp(distance / light.radius.x, 0.0, 1.0);
	if (attenuation > 0.0f) {
		vec3  incident = normalize(lightVec);
		vec3 halfDir = normalize(incident + viewDir);

		float lambert = clamp(dot(incident, normal), 0.0, 1.0);

		float rFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
		float sFactor = calculateSpecular(rFactor) * specSample;

		vec3 attenuated = light.colour.rgb * attenuation;
		//fragColor.rgb += albedo.rgb * attenuated * lambert; //diffuse light
		//fragColor.rgb += albedo.rgb * attenuated * lambert;
		//fragColor.rgb += light.colour.rgb * attenuated * sFactor * 0.33;
		diffuseOut.rgb += attenuated * lambert; //diffuse light
		specularOut.rgb += attenuated * sFactor * 0.33; //specular light
	}
}