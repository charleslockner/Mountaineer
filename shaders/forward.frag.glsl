
uniform bool uHasNormals;
uniform bool uHasColors;
uniform bool uHasTextures;
uniform bool uHasTansAndBitans;

uniform vec3 uLights[40]; // max 10 lights each with position, direction, color, attributes
uniform vec3 uCameraPosition;
uniform sampler2D uTexture;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
varying vec3 vVertexColor;
varying vec2 vVertexUV;

void main(void) {
   vec3 lightPosition, lightDirection, lightColor, lightAttr;
   vec3 skinColor, normal, reflection, view;
   vec3 ambient, diffuse, specular, finColor;
   float specDot, lightStrength, lightAttenuation, lightRadius, lightDistance, illumination;
   float shine;

   if (uHasColors && uHasTextures)
      skinColor = vec3(texture2D(uTexture, vVertexUV)) * vVertexColor;
   else if (uHasColors)
      skinColor = vVertexColor;
   else if (uHasTextures)
      skinColor = vec3(texture2D(uTexture, vVertexUV));
   else
      skinColor = vec3(0.8, 0.7, 0.3);

   if (!uHasNormals)
      finColor = skinColor;
   else {
      normal = normalize(vWorldNormal);
      view = normalize(vWorldPosition - uCameraPosition);
      shine = 500.0;

      ambient = skinColor * 0.25;
      finColor = vec3(0.0);

      // loop through each light
      for (int i = 0; i < 1; i++) {
         lightPosition = uLights[4*i];
         lightColor = uLights[4*i+2];
         lightAttr = uLights[4*i+3];
         lightStrength = lightAttr.x;

         lightDirection = normalize(vWorldPosition - lightPosition);
         reflection = normalize(2.0 * normal * dot(normal, lightDirection) - lightDirection);
         lightDistance = length(vWorldPosition - lightPosition);
         illumination = lightStrength / (1.0 + 2.0*lightDistance + lightDistance*lightDistance);

         diffuse = skinColor * dot(normal, -lightDirection);
         specular = skinColor * pow(max(0.0, dot(reflection, view)), shine);
         finColor += illumination * lightColor * (specular + diffuse + ambient);
      }
   }


   gl_FragColor = vec4(finColor, 1.0);
}