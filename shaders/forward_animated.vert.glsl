
uniform mat4 uModelM;
uniform mat4 uProjViewM;
uniform mat4 uBoneMs[100];

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec3 aColor;
attribute vec2 aUV;

attribute vec4 aBoneIndices0;
attribute vec4 aBoneIndices1;
attribute vec4 aBoneIndices2;
attribute vec4 aBoneIndices3;
attribute vec4 aBoneWeights0;
attribute vec4 aBoneWeights1;
attribute vec4 aBoneWeights2;
attribute vec4 aBoneWeights3;
attribute float aNumInfluences;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
varying vec3 vColor;
varying vec2 vUV;

void main(void) {
   int index;
   float weight;

   mat4 animMatrix = mat4(0.0);
   int numInfluences = int(aNumInfluences);

   for (int i = 0; i < numInfluences; i++) {
      if (i < 4) {
         index = int(aBoneIndices0[i]);
         weight = aBoneWeights0[i];
      } else if (i < 8) {
         index = int(aBoneIndices1[i-4]);
         weight = aBoneWeights1[i-4];
      } else if (i < 12) {
         index = int(aBoneIndices2[i-8]);
         weight = aBoneWeights2[i-8];
      } else {
         index = int(aBoneIndices3[i-12]);
         weight = aBoneWeights3[i-12];
      }

      animMatrix += weight * uBoneMs[index];
   }

   mat4 modelM = uModelM * animMatrix;

   vWorldPosition = vec3(modelM * vec4(aPosition, 1.0));
   vWorldNormal = vec3(modelM * vec4(aNormal, 0.0));
   vColor = aColor;
   vUV = aUV;

   gl_Position = uProjViewM * vec4(vWorldPosition, 1.0);
}