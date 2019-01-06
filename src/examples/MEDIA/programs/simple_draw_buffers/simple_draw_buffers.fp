!!ARBfp1.0
OPTION NV_fragment_program2;
OPTION ATI_draw_buffers; 

ATTRIB pos = fragment.texcoord[1]; # fragment position
ATTRIB normal = fragment.texcoord[2]; # fragment normal

PARAM shininess = 60.0;
PARAM ambientColor = { 0.1, 0.1, 0.1, 1.0 };
PARAM lightPos = { 10.0, 10.0, 0.0 };
PARAM lightColor = { 0.9, 0.9, 0.9, 1.0 };

SHORT TEMP color, lit;
SHORT TEMP N, V, L, H, att;

# normalize normal and view vector
NRM N.xyz, normal;
NRM V.xyz, -pos; # eye pos is (0, 0, 0) in eye space

MOV color, ambientColor;

# compute local light vector
ADD L.xyz, lightPos, -pos;

# attenuation
#DP3 att.x, L, L; # |L|^2
#MAD att.x, lightPos.a, att.x, 1.0;
#RCP att.x, att.x; # 1 / (1 + |L|^2*K)

NRM L.xyz, L;

# compute half-angle vector
ADD H.xyz, L, V;
NRM H.xyz, H;

# calculate lighting
DP3 lit.x, N, L;
DP3 lit.y, N, H;
MOV lit.w, shininess;
LIT lit, lit;

#MUL lit.yz, lit, att.x;

# add colors
MAD color, lit.y, lightColor, color; # diffuse
ADD color, lit.z, color; # specular (white)

# write outputs
MOV result.color[0], color;
MOV result.color[1], N;
MOV result.color[2], pos;
MOV result.color[3], H;
END
