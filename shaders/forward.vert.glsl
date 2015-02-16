
uniform bool uHasTextures;
uniform bool uHasAnimations;

uniform mat4 uModelMatrix;
uniform mat4 uProjViewMatrix;
uniform mat4 uBoneMatrices[100];

attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;
attribute vec3 aVertexColor;
attribute vec2 aVertexUV;

attribute vec4 aVertexBoneIndices0;
attribute vec4 aVertexBoneIndices1;
attribute vec4 aVertexBoneIndices2;
attribute vec4 aVertexBoneIndices3;
attribute vec4 aVertexBoneWeights0;
attribute vec4 aVertexBoneWeights1;
attribute vec4 aVertexBoneWeights2;
attribute vec4 aVertexBoneWeights3;
attribute float aNumInfluences;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
varying vec3 vVertexColor;
varying vec2 vVertexUV;

void main(void) {
   mat4 animMatrix;
   int index, numInfluences;
   float weight;

   if (uHasAnimations) {
      animMatrix = mat4(0.0);
      numInfluences = int(aNumInfluences);

      for (int i = 0; i < numInfluences; i++) {
         if (i < 4) {
            index = int(aVertexBoneIndices0[i]);
            weight = aVertexBoneWeights0[i];
         } else if (i < 8) {
            index = int(aVertexBoneIndices1[i-4]);
            weight = aVertexBoneWeights1[i-4];
         } else if (i < 12) {
            index = int(aVertexBoneIndices2[i-8]);
            weight = aVertexBoneWeights2[i-8];
         } else {
            index = int(aVertexBoneIndices3[i-12]);
            weight = aVertexBoneWeights3[i-12];
         }

         animMatrix += weight * uBoneMatrices[index];
      }
   } else
      animMatrix = mat4(1.0);

   mat4 modelM = uModelMatrix * animMatrix;

   vWorldPosition = vec3(modelM * vec4(aVertexPosition, 1.0));
   vWorldNormal = vec3(modelM * vec4(aVertexNormal, 0.0));
   vVertexColor = aVertexColor;
   vVertexUV = aVertexUV;

   gl_Position = uProjViewMatrix * vec4(vWorldPosition, 1.0);
}