
uniform bool uHasTextures;
uniform bool uHasAnimations;

uniform mat4 uModelMatrix;
uniform mat4 uProjViewMatrix;
uniform mat4 uBoneMatrices[100];

attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;
attribute vec3 aVertexColor;
attribute vec2 aVertexUV;

attribute vec4 bIndices0;
attribute vec4 bIndices1;
attribute vec4 bIndices2;
attribute vec4 bIndices3;
attribute vec4 bWeights0;
attribute vec4 bWeights1;
attribute vec4 bWeights2;
attribute vec4 bWeights3;
attribute float aInfluences;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
varying vec3 vVertexColor;
varying vec2 vVertexUV;

void main(void) {
   mat4 animMatrix;
   int index, numInfluences;
   float weight;

   if (!uHasAnimations)
      animMatrix = mat4(1.0);
   else {
      animMatrix = mat4(0.0);
      numInfluences = int(aInfluences);

      for (int i = 0; i < numInfluences; i++) {
         if (i < 4) {
            index = int(bIndices0[i]);
            weight = bWeights0[i];
         } else if (i < 8) {
            index = int(bIndices1[i-4]);
            weight = bWeights1[i-4];
         } else if (i < 12) {
            index = int(bIndices2[i-8]);
            weight = bWeights2[i-8];
         } else {
            index = int(bIndices3[i-12]);
            weight = bWeights3[i-12];
         }

         animMatrix += weight * uBoneMatrices[index];
      }
   }

   mat4 modelM = uModelMatrix * animMatrix;

   vWorldPosition = vec3(modelM * vec4(aVertexPosition, 1.0));
   vWorldNormal = vec3(modelM * vec4(aVertexNormal, 0.0));
   vVertexColor = aVertexColor;
   vVertexUV = aVertexUV;

   gl_Position = uProjViewMatrix * vec4(vWorldPosition, 1.0);
}