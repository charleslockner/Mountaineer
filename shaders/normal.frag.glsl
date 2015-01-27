
varying vec3 vWorldNormal;

void main(void) {
   gl_FragColor = vec4(normalize(vWorldNormal), 1.0);
}