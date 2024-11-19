#version 330 core
out vec4 oFragColor;

in vec4 iVertexColor;
in vec2 iTexCoord;
in vec3 iNormal;
in vec3 iFragPos;

struct Material {
    sampler2D texture_diffuse;
    sampler2D texture_specular;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 1

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

vec3 CalcDirLight(DirLight light, vec3 iNormal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 iNormal, vec3 iFragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 iNormal, vec3 iFragPos, vec3 viewDir);

void main()
{
    oFragColor = vec4(iTexCoord, 1, 1.0);

    vec3 norm = normalize(iNormal);
    vec3 viewDir = normalize(viewPos - iFragPos);

    vec3 result;
    result += CalcDirLight(dirLight, norm, viewDir);
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, iFragPos, viewDir);
    result += CalcSpotLight(spotLight, norm, iFragPos, viewDir);

    oFragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 iNormal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(iNormal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, iNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse, iTexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse, iTexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular, iTexCoord));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 iNormal, vec3 iFragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - iFragPos);
    // diffuse shading
    float diff = max(dot(iNormal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, iNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - iFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse, iTexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse, iTexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular, iTexCoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 iNormal, vec3 iFragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - iFragPos);
    // diffuse shading
    float diff = max(dot(iNormal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, iNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - iFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse, iTexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse, iTexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular, iTexCoord));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
