
uniform mat4 uModelM;
uniform mat4 uProjViewM;

attribute vec3 aPosition;
attribute vec3 aTangent;
attribute vec3 aBitangent;
attribute vec3 aNormal;
attribute vec3 aColor;
attribute vec2 aUV;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
varying vec3 vWorldTangent;
varying vec3 vWorldBitangent;
varying vec3 vColor;
varying vec2 vUV;

void main(void) {
   vWorldPosition = vec3(uModelM * vec4(aPosition, 1.0));
   vWorldTangent = vec3(uModelM * vec4(aTangent, 0.0));
   // vec3(inverse(transpose(uModelM)) * vec4(aTangent, 0.0));

   vWorldBitangent = vec3(uModelM * vec4(aBitangent, 0.0));
   vWorldNormal = vec3(uModelM * vec4(aNormal, 0.0));
   vColor = aColor;
   vUV = aUV;

   gl_Position = uProjViewM * vec4(vWorldPosition, 1.0);
}