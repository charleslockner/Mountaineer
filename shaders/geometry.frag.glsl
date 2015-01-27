
uniform sampler2D uTexture;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
varying vec2 vTextureCoord;

void main(void) {
   vec3 textureColor;

   textureColor = vec3(texture2D(uTexture, vTextureCoord));

   gl_FragData[0] = vec4(vWorldPosition, 1.0);
   gl_FragData[1] = vec4(vWorldNormal, 1.0);
   gl_FragData[2] = vec4(textureColor, 1.0);
}