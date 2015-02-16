
uniform bool uHasTextures;
uniform bool uHasBones;
uniform bool uHasAnimations;

uniform mat4 uModelMatrix;
uniform mat4 uProjViewMatrix;
uniform mat4 uBoneMatrices[100];

attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;
attribute vec3 aVertexColor;
attribute vec2 aVertexUV;
attribute vec4 aVertexBoneIndices;
attribute vec4 aVertexBoneWeights;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
varying vec3 vVertexColor;
varying vec2 vVertexUV;

void main(void) {
   mat4 animMatrix;

   if (uHasBones && uHasAnimations) {
      animMatrix = mat4(0.0);
      for (int i = 0; i < 4; i++)
         animMatrix += aVertexBoneWeights[i] * uBoneMatrices[int(aVertexBoneIndices[i])];
   }
   else
      animMatrix = mat4(1.0);

   mat4 modelM = uModelMatrix * animMatrix;

   vWorldPosition = vec3(modelM * vec4(aVertexPosition, 1.0));
   vWorldNormal = vec3(modelM * vec4(aVertexNormal, 0.0));
   vVertexColor = aVertexColor;
   vVertexUV = aVertexUV;

   gl_Position = uProjViewMatrix * vec4(vWorldPosition, 1.0);
}