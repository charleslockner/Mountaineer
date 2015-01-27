uniform mat4 uModelMatrix;
uniform mat4 uProjViewMatrix;

attribute vec3 aVertexPosition;
attribute vec2 aTextureCoord;

varying vec2 vTextureCoord;

void main() {
   vTextureCoord = aTextureCoord;

   gl_Position = uProjViewMatrix * uModelMatrix * vec4(aVertexPosition, 1.0);
}