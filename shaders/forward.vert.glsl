
uniform bool uHasBones;
uniform bool uHasAnimations;

uniform mat4 uModelMatrix;
uniform mat4 uProjViewMatrix;
uniform mat4 uBoneMatrices[100];

attribute vec3 aVertexPosition;
attribute vec3 aVertexColor;
attribute vec3 aVertexNormal;
attribute vec2 aTextureCoord;
attribute vec4 aBoneIndices;
attribute vec4 aBoneWeights;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
varying vec3 vVertexColor;
varying vec2 vTextureCoord;

void main(void) {
   mat4 animMatrix;

   // if (uHasBones && uHasAnimations) {
   //    animMatrix = mat4(0.0);
   //    for (int i = 0; i < 4; i++)
   //       animMatrix += aBoneWeights[i] * uBoneMatrices[int(aBoneIndices[i])];
   // }
   // else
      animMatrix = mat4(1.0);

   mat4 modelM = uModelMatrix * animMatrix;

   vWorldPosition = vec3(modelM * vec4(aVertexPosition, 1.0));
   vWorldNormal = vec3(modelM * vec4(aVertexNormal, 0.0));
   vVertexColor = aVertexColor;
   vTextureCoord = aTextureCoord;

   gl_Position = uProjViewMatrix * vec4(vWorldPosition, 1.0);
}