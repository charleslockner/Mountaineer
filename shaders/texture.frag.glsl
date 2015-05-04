
uniform sampler2D uTexture;
varying vec2 vUV;

void main(void) {
   vec3 pixColor = vec3(texture2D(uTexture, vUV));

   gl_FragColor = vec4(pixColor, 1.0);
}