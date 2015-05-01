
uniform bool uHasNormals;
uniform bool uHasColors;
uniform bool uHasTexture;
uniform bool uHasNormalMap;
uniform bool uHasSpecularMap;

uniform vec3 uLights[40]; // max 10 lights each with position, direction, color, attributes
uniform vec3 uCameraPosition;
uniform sampler2D uTexture;
uniform sampler2D uNormalMap;
uniform sampler2D uSpecularMap;

varying vec3 vWorldPosition;
varying vec3 vWorldNormal;
varying vec3 vWorldTangent;
varying vec3 vWorldBitangent;
varying vec3 vColor;
varying vec2 vUV;

void main(void) {
   mat3 TBN;
   vec3 lightPosition, lightDirection, lightColor, lightAttr;
   vec3 skinColor, normal, reflection, viewDirection;
   vec3 ambient, diffuse, specular, finalColor;
   float specDot, lightStrength, lightAttenuation, lightRadius, lightDistance, illumination;
   float shine = 400.0;

   if (uHasColors && uHasTexture)
      skinColor = vec3(texture2D(uTexture, vUV)) * vColor;
   else if (uHasColors)
      skinColor = vColor;
   else if (uHasTexture)
      skinColor = vec3(texture2D(uTexture, vUV));
   else
      skinColor = vec3(0.8, 0.7, 0.3);

   if (uHasNormals) {
      viewDirection = normalize(vWorldPosition - uCameraPosition);

      if (uHasNormalMap) {
         mat3 TBN = mat3(normalize(vWorldTangent), normalize(vWorldBitangent), normalize(vWorldNormal));
         normal = TBN * normalize(texture2D(uNormalMap, vUV).rgb * 2.0 - 1.0);
      } else
         normal = normalize(vWorldNormal);

      ambient = skinColor * 0.25;
      finalColor = vec3(0.0);

      // loop through each light
      for (int i = 0; i < 2; i++) {
         // lightPosition = uLights[4*i];
         lightDirection = uLights[4*i+1];
         lightColor = uLights[4*i+2];
         // lightAttr = uLights[4*i+3];
         // lightStrength = lightAttr.x;

         // lightDirection = normalize(vWorldPosition - lightPosition);

         reflection = normalize(2.0 * normal * dot(normal, lightDirection) - lightDirection);
         // lightDistance = length(vWorldPosition - lightPosition);
         // illumination = lightStrength / (1.0 + 2.0*lightDistance + lightDistance*lightDistance);

         diffuse = skinColor * clamp(dot(normal, -lightDirection), 0.0, 1.0);
         specular = skinColor * pow(clamp(dot(reflection, viewDirection), 0.0, 1.0), shine);
         // finalColor += illumination * lightColor * (specular + diffuse + ambient);
         finalColor += lightColor * (specular + diffuse + ambient);
      }
   } else
      finalColor = skinColor;

   gl_FragColor = vec4(finalColor, 1.0);
}