!!ARBfp1.0
#
# NV3X shader 
# 8 estimation samples
# 64 total samples
#

OPTION NV_fragment_program;

PARAM	fwidth = program.local[0];	# { 0, 0, 0, fwidth }
PARAM	jxyscale = program.local[1];
PARAM	oneover64 = 0.015625;

TEMP	smcoord;

SHORT TEMP	spot, decal, amb;
SHORT TEMP	fsize, jcoord, shadow, tmp, jitter, c;
SHORT TEMP	normal, lightv, view, half, refl, diffuse, spec;

# some setup
MUL	fsize, fragment.texcoord[1].w, fwidth;
MUL	jcoord.xyz, fragment.position, jxyscale;	# assumes that z is set to 0, too
MOV	shadow, 0;

# samples 1/2

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xyzw, jitter.xyxx, fsize.wwxx, fragment.texcoord[1];	# also sets zw to fragment.texcoord[1].zw
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 3/4

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 5/6

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 7/8

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 9/10

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 11/12

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 13/14

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 15/16

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 17/18

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 19/20

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 21/22

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 23/24

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 25/26

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 27/28

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 29/30

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 31/32

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 33/34

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 35/36

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 37/38

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 39/40

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 41/42

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 43/44

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 45/46

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 47/48

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 49/50

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 51/52

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 53/54

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 55/56

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 57/58

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 59/60

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 61/62

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

# samples 63/64

TEX	jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADD	jcoord.z, jcoord.z, 0.03125;#0.0078125;	# increment lookup coord

MAD	smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;

MAD	smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP	tmp, smcoord, texture[1], 2D;	# sample shadowmap
MADX	shadow, tmp, oneover64, shadow;
	
# normalize normal
DP3		normal, fragment.texcoord[2], fragment.texcoord[2];
RSQ		normal.w, normal.w;
MUL		normal.xyz, fragment.texcoord[2], normal.w;

# normalize light vector
DP3		lightv, fragment.texcoord[3], fragment.texcoord[3];
RSQ		lightv.w, lightv.w;
MUL		lightv.xyz, fragment.texcoord[3], lightv.w;

# normalize view vector
DP3		view, fragment.texcoord[4], fragment.texcoord[4];
RSQ		view.w, view.w;
MUL		view.xyz, fragment.texcoord[4], view.wwww;

# reflect view vector around normal
RFLH		refl.xyz, normal, view;

# calculate diffuse and specular terms
DP3X_SAT	diffuse.w, lightv, normal;
DP3X_SAT	spec.w, refl, lightv;
POWH		spec, spec.w, 64.0;

TXP		spot, fragment.texcoord[1], texture[3], 2D;
TEX		decal, fragment.texcoord[0], texture[0], 2D;
MULX	shadow, shadow, spot;
MULX	diffuse, diffuse.w, decal;
MULX	diffuse, diffuse, shadow;
MADX	diffuse, spec, shadow, diffuse;

MADX	result.color, decal, 0.1, diffuse;

END
