#version 150

in vec4 col;
in vec2 tc;

out vec4 c;
uniform sampler2D textureImage;

void main()
{
  // compute the final pixel color

  //c = col;
  c = texture(textureImage, tc);
}

