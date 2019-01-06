!!ARBfp1.0

PARAM constants = { 3, 0, 0, 0 };

TEMP decalTexture;

# Do the texture lookup
TEX decalTexture, fragment.texcoord, texture[0], 2D;

# Multiply by a constant to brighten
MUL decalTexture, decalTexture, constants.x;

# Multiply decal texture and vertex color together and output
MUL result.color, decalTexture, fragment.color;

END
