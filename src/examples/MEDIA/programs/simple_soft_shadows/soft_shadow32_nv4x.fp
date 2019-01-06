!!ARBfp1.0
#
# NV4X shader 
# 4 estimation samples
# 32 total samples
#

OPTION NV_fragment_program2;

PARAM	filtersize = program.local[0];
PARAM	jxyscale = program.local[1];
PARAM	oneover4 = 0.25;
PARAM	oneover32 = 0.03125;

TEMP	smcoord;

SHORT TEMP	fsize, jcoord, shadow, jitter, c;
SHORT TEMP	normal, lightv, view, half, diffuse, refl, spec;
SHORT TEMP	spot, decal;

# some setup
MUL		fsize.w, fragment.texcoord[1].w, filtersize.w;
MOV		smcoord.zw, fragment.texcoord[1];
MUL		jcoord.xyz, fragment.position, jxyscale;	# assumes that z is set to 0, too
MOV		shadow.w, 0;

# perform 4 'test' samples

# samples 1/2

TEX		jitter, jcoord, texture[2], 3D;	# lookup jitter map
ADDH	jcoord.z, jcoord.z, 0.0625;	# increment lookup coord

MAD		smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP		shadow.x, smcoord, texture[1], 2D;	# sample shadowmap

MAD		smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP		shadow.y, smcoord, texture[1], 2D;	# sample shadowmap

DP2AX	shadow.w, shadow, oneover4, shadow.w;

# samples 3/4

TEX		jitter, jcoord, texture[2], 3D;	# lookup jitter map

MAD		smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
TXP		shadow.x, smcoord, texture[1], 2D;	# sample shadowmap

MAD		smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
TXP		shadow.y, smcoord, texture[1], 2D;	# sample shadowmap

DP2AX	shadow.w, shadow, oneover4, shadow.w;

# normalize vectors
NRMH	normal, fragment.texcoord[2];
NRMH	lightv, fragment.texcoord[3];
NRMH	view, fragment.texcoord[4];

# diffuse dot product
DP3XC_SAT	diffuse.w, lightv, normal;

SUBX		c.w, shadow.w, 1;
MULX		c.w, c.w, shadow.w;
MULXC		c.w, c.w, diffuse.w;

IF NE.w;	# oversample only across shadow edge

	MUL		shadow.w, shadow, oneover4;
	ADDH	jcoord.z, jcoord.z, 0.0625;	# increment lookup coord
	
	REP	14;	# the rest 28 samples ...

		TEX		jitter, jcoord, texture[2], 3D;	# lookup jitter map
		ADDH	jcoord.z, jcoord.z, 0.0625;	# increment lookup coord

		MAD		smcoord.xy, jitter.xyxx, fsize.w, fragment.texcoord[1];
		TXP		shadow.x, smcoord, texture[1], 2D;	# sample shadowmap

		MAD		smcoord.xy, jitter.zwxx, fsize.w, fragment.texcoord[1];
		TXP		shadow.y, smcoord, texture[1], 2D;	# sample shadowmap

		DP2AX	shadow.w, shadow, oneover32, shadow.w;

	ENDREP;

ENDIF;

RFLH	refl.xyz, normal, view;

# calculate specular term
DP3X_SAT	spec.w, refl, lightv;
POW		spec, spec.w, 64.0;

TXP		spot, fragment.texcoord[1], texture[3], 2D;
TEX		decal, fragment.texcoord[0], texture[0], 2D;
MULX	shadow, shadow.w, spot;
MULX	diffuse, diffuse.w, decal;
MULX	diffuse, diffuse, shadow;
MADX	diffuse, spec, shadow, diffuse;

MADX	result.color, decal, 0.1, diffuse;

END
