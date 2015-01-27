attribute vec2 aScreenPosition;

varying vec2 vTextureCoord;

void main() {
   gl_Position = vec4(aScreenPosition, 0.0, 1.0);
   vTextureCoord = aScreenPosition * 0.5 + 0.5;
}