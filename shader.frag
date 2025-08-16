#version 330

in vec2 frag_tex_coord;
in vec4 frag_color;

out vec4 finalColor;

uniform vec2 resolution;
uniform float focal_length;

#define MAX_CIRCLES 10
uniform vec3 positions[MAX_CIRCLES];
uniform vec3 colors[MAX_CIRCLES];
uniform float radii[MAX_CIRCLES];
uniform int circle_count;

void main() {
    // make the center of the window 0, 0
    vec2 pos = (2.0 * gl_FragCoord.xy - resolution.xy) / resolution.y;

    // float fov = 90.0;
    // float focal_length = 0.5/tan(fov/2.0);
    vec3 ray = normalize(vec3(pos, focal_length));
    vec3 origin = vec3(0., 0., 0.);

    finalColor = vec4(0.0, 0.0, 0.0, 0.0);

    float min_dist = 1.0 / 0.0; // infinity
    for (int i = 0; i < circle_count; i++) {
        vec3 center = positions[i];
        float radius = radii[i];

        // behind the camera
        if (center.z < -radius / 2.) continue;

        vec3 oc = center - origin;
        float a = dot(ray, ray);
        float b = -2.0 * dot(ray, oc);
        float c = dot(oc, oc) - radius * radius;
        float d = b*b - 4*a*c;

        if (d >= 0) {
            float sqrt_d = sqrt(d);
            float t1 = (-b + sqrt_d) / (2.0 * a);
            float t2 = (-b - sqrt_d) / (2.0 * a);
            float min_t = min(t1, t2);

            if (min_t < min_dist && min_t >= 0) {
                finalColor = vec4(colors[i], 1.0);
                min_dist = min_t;
            }
        }
    }
}
