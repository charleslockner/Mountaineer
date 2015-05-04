
uniform mat4 uProjViewModelM;
attribute vec3 aPosition;
attribute vec2 aUV;

varying vec2 vUV;

void main(void) {
   vUV = aUV;
   gl_Position = uProjViewModelM * vec4(aPosition, 1.0);
}