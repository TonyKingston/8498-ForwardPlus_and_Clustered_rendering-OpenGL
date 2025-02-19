#pragma once

#define CLUSTER_DEBUG 1 // Show cluster slices

// Colours for tile debugging
// Blue to red, then light gray
vec3 debugColours[6] = vec3[](
	vec3(0, 0, 1), vec3(0.2, 0.68, 1), vec3(0, 1, 0),
	vec3(0.95, 0.95, 0.3), vec3(1, 0, 0), vec3(0.8, 0.8, 0.8)
);

// Colours for cluster slices.
vec3 clusterColours[8] = vec3[](
	vec3(0, 0, 0), vec3(0, 0, 1), vec3(0, 1, 0), vec3(0, 1, 1),
	vec3(1, 0, 0), vec3(1, 0, 1), vec3(1, 1, 0), vec3(1, 1, 1)
);

// Get debug colour for a tile depending on how many lights are in it.
vec3 getDebugColour(uint lightCount) {
	// Normalise in [0, 1] range
	float t = clamp(lightCount / 100.0, 0.0, 1.0);
	vec3 colour;
	colour = mix(debugColours[0], debugColours[1], smoothstep(0.0, 0.1, t));  // 0-10 lights
	colour = mix(colour, debugColours[2], smoothstep(0.1, 0.3, t));    // 10-30 lights
	colour = mix(colour, debugColours[3], smoothstep(0.3, 0.6, t));    // 30-60 lights
	colour = mix(colour, debugColours[4], smoothstep(0.6, 1.0, t));    // 60-100 lights
	colour = mix(colour, debugColours[5], smoothstep(1.0, 1.1, t));    // 100+ lights
	return colour;
}

vec3 getDebugColourGrayScale(uint lightCount) {
	return vec3(float(lightCount) / float(3));
}