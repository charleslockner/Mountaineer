
uniform mat4 uModelMatrix;
uniform mat4 uProjViewMatrix;
uniform mat4 uBoneTransforms[100];

attribute vec3 aVertexPosition;
// attribute vec3 aVertexColor;
attribute vec3 aVertexNormal;
// attribute vec2 aTextureCoord;
attribute vec4 aBoneIndices;
attribute vec4 aBoneWeights;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
// varying vec3 vVertexColor;
// varying vec2 vTextureCoord;

void main(void) {
   mat4 animBoneM0 = aBoneWeights.w * uBoneTransforms[int(aBoneIndices.w)];
   mat4 animBoneM1 = aBoneWeights.x * uBoneTransforms[int(aBoneIndices.x)];
   mat4 animBoneM2 = aBoneWeights.y * uBoneTransforms[int(aBoneIndices.y)];
   mat4 animBoneM3 = aBoneWeights.z * uBoneTransforms[int(aBoneIndices.z)];
   mat4 animMatrix = animBoneM0 + animBoneM1 + animBoneM2 + animBoneM3;
   // animMatrix = uBoneTransforms[int(aBoneIndices.w)];
   mat4 modelAnimM = uModelMatrix * animMatrix;

   vWorldPosition = vec3(modelAnimM * vec4(aVertexPosition, 1.0));
   vWorldNormal = vec3(modelAnimM * vec4(aVertexNormal, 0.0));
   // vVertexColor = aVertexColor;
   // vTextureCoord = aTextureCoord;

   gl_Position = uProjViewMatrix * vec4(vWorldPosition, 1.0);
}