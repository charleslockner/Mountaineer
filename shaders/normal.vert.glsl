
uniform mat4 uModelMatrix;
uniform mat4 uProjViewMatrix;

attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;

varying vec3 vWorldNormal;

void main(void) {
   gl_Position = uProjViewMatrix * uModelMatrix * vec4(aVertexPosition, 1.0);
   vWorldNormal = vec3(uModelMatrix * vec4(normalize(aVertexNormal), 0.0));
}