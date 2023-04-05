#version 330 compatibility
layout(location = 0) out vec4 color;

in vec3 vertColorFrag;
in vec3 vertFragPos;
in vec3 vertNormal;
in vec2 vertUVLayer;

uniform mat4 view;
uniform mat4 projection;

uniform int diffuseLoaded;
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D glossTexture;
uniform sampler2D specularTexture;

// http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
  // get edge vectors of the pixel triangle
  vec3 dp1 = dFdx(p);
  vec3 dp2 = dFdy(p);
  vec2 duv1 = dFdx(uv);
  vec2 duv2 = dFdy(uv);

  // solve the linear system
  vec3 dp2perp = cross(dp2, N);
  vec3 dp1perp = cross(N, dp1);
  vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  // construct a scale-invariant frame 
  float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
  return mat3(T * invmax, B * invmax, N);
}

vec3 perturb_normal(vec3 N, vec3 V, vec2 texcoord)
{
  // assume N, the interpolated vertex normal and 
  // V, the view vector (vertex to eye)
  vec3 map = normalize(texture(normalTexture, texcoord).rgb * -2 + 1);
  // not deriving z for now
  mat3 TBN = cotangent_frame(N, -V, texcoord);
  return normalize(TBN * map);
}

void main()
{
  // Ambient
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * vec3(1, 1, 1);  // Amb color

  vec3 viewPos = inverse(view)[3].xyz;
  vec3 viewDir = normalize(viewPos - vertFragPos);
  
  vec3 norm = normalize(vertNormal);
  if (diffuseLoaded == 1) {
    norm = perturb_normal(norm, viewDir, vertUVLayer);
  }

  // Diffuse
  vec3 lightDir = normalize(inverse(view)[3].xyz - vertFragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * texture(diffuseTexture, vertUVLayer).rgb;  // Light color
  float alpha = texture(diffuseTexture, vertUVLayer).a;

  // Specular
  vec3 reflectDir = reflect(-lightDir, norm);
  
  float gloss = texture(glossTexture,vertUVLayer).r;
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
  vec3 specular = texture(specularTexture, vertUVLayer).rgb * spec * gloss;
  
  // Result
  if (diffuseLoaded == 1) {
    vec4 result = vec4(ambient + specular + diffuse, alpha);
    color = result;
    if (color.a == 0)
      discard;
  } else {
    color = vec4(diff);
  }
}