
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
         // tangent and bitangent look messed up
         mat3 m = mat3(normalize(vWorldTangent), normalize(vWorldBitangent), normalize(vWorldNormal));
         // TBN = mat3(m[0][0], m[1][0], m[2][0],
         //            m[0][1], m[1][1], m[2][1],
         //            m[0][2], m[1][2], m[2][2]);
         // TBN = mat3(m[0][0], m[0][1], m[0][2],
         //            m[1][0], m[1][1], m[1][2],
         //            m[2][0], m[2][1], m[2][2]);
         // viewDirection = TBN * viewDirection;
         normal = TBN * normalize(texture2D(uNormalMap, vUV).rgb*2.0 - 1.0);
      } else
         normal = normalize(vWorldNormal);

      ambient = skinColor * 0.25;
      finalColor = vec3(0.0);

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

         diffuse = skinColor * clamp(dot(normal, -lightDirection), 0.0, 1.0);
         specular = skinColor * pow(clamp(dot(reflection, viewDirection), 0.0, 1.0), shine);
         finalColor += illumination * lightColor * (specular + diffuse + ambient);
      }
   } else
      finalColor = skinColor;


   gl_FragColor = vec4(normal, 1.0);
}