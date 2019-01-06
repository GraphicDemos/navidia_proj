!!ARBfp1.0
# multiple lights using looping
# light positions and colors stored in texture

OPTION NV_fragment_program2;

ATTRIB pos = fragment.texcoord[1]; # fragment position
ATTRIB normal = fragment.texcoord[2]; # fragment normal

PARAM shininess = 60.0;
PARAM ambientColor = { 0.1, 0.1, 0.1, 1.0 };
PARAM nlights = program.local[0]; # number of lights
PARAM eye_pos = program.local[1];

SHORT TEMP lightPos, lightColor;
SHORT TEMP lightIndex, color, lit;
SHORT TEMP N, V, L, H, att;

# normalize normal and view vector
NRM N.xyz, normal;
NRM V.xyz, -pos; # eye pos is (0, 0, 0) in eye space

# loop over lights
MOV color, ambientColor;
MOV lightIndex.x, 0.0;
REP nlights;
    # get light position and color from texture
    TEXC lightPos, lightIndex, texture[0], RECT;  # write condition code
    TEX lightColor, lightIndex, texture[1], RECT;
    # call correct lighting function based on w component of position
    IF EQ.w;          # lightPos.w == 0.0
      CAL dirlight;
    ELSE;
	  CAL pointlight;
	ENDIF;
	ADD lightIndex, lightIndex, 1.0; # increment loop counter
ENDREP;
MOV result.color, color;
RET;

# point light function
pointlight:
# compute local light vector
ADD L.xyz, lightPos, -pos;

# attenuation
DP3 att.x, L, L; # |L|^2
MAD att.x, lightColor.a, att.x, 1.0;
RCP att.x, att.x; # 1 / (1 + |L|^2*K)

NRM L.xyz, L;

# compute half-angle vector
ADD H.xyz, L, V;
NRM H.xyz, H;

# calculate lighting
DP3 lit.x, N, L;
DP3 lit.y, N, H;
MOV lit.w, shininess;
LIT lit, lit;

MUL lit.yz, lit, att.x;

# add colors
MAD color, lit.y, lightColor, color; # diffuse
MAD color, lit.z, lightColor, color; # specular
#ADD color, lit.z, color; # specular (white)
RET;


# directional light function
dirlight:
# compute half-angle vector
ADD H.xyz, lightPos, V;
NRM H.xyz, H;

# calculate lighting
DP3 lit.x, N, lightPos;
DP3 lit.y, N, H;
MOV lit.w, shininess;
LIT lit, lit;

# add colors
MAD color, lit.y, lightColor, color; # diffuse
MAD color, lit.z, lightColor, color; # specular
#ADD color, lit.z, color; # specular (white)
RET;

END