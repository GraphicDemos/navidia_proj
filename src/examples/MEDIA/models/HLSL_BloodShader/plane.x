xof 0303txt 0032
template XSkinMeshHeader {
 <3cf169ce-ff7c-44ab-93c0-f78f62d172e2>
 WORD nMaxSkinWeightsPerVertex;
 WORD nMaxSkinWeightsPerFace;
 WORD nBones;
}

template VertexDuplicationIndices {
 <b8d65549-d7c9-4995-89cf-53a9a8b031e3>
 DWORD nIndices;
 DWORD nOriginalVertices;
 array DWORD indices[nIndices];
}

template SkinWeights {
 <6f0d123b-bad2-4167-a0d0-80224f25fabb>
 STRING transformNodeName;
 DWORD nWeights;
 array DWORD vertexIndices[nWeights];
 array FLOAT weights[nWeights];
 Matrix4x4 matrixOffset;
}


Frame SCENE_ROOT {
 

 FrameTransformMatrix {
  1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,-1.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
 }

 Frame groundPlane_transform {
  

  FrameTransformMatrix {
   12.000000,0.000000,0.000000,0.000000,0.000000,0.000000,12.000000,0.000000,0.000000,-1.000000,0.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
  }
 }

 Frame persp {
  

  FrameTransformMatrix {
   0.999701,-0.000000,0.024432,0.000000,0.024149,-0.151922,-0.988097,0.000000,0.003712,0.988392,-0.151877,0.000000,0.008523,2.269406,-0.348719,1.000000;;
  }
 }

 Frame top {
  

  FrameTransformMatrix {
   1.000000,0.000000,0.000000,0.000000,0.000000,0.000000,-1.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,100.000000,0.000000,1.000000;;
  }
 }

 Frame front {
  

  FrameTransformMatrix {
   1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,100.000000,1.000000;;
  }
 }

 Frame side {
  

  FrameTransformMatrix {
   0.000000,0.000000,-1.000000,0.000000,0.000000,1.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,100.000000,0.000000,0.000000,1.000000;;
  }
 }

 Frame pPlane1 {
  

  FrameTransformMatrix {
   1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
  }

  Mesh pPlaneShape1 {
   121;
   -0.500000;-0.000000;0.500000;,
   -0.400000;-0.000000;0.500000;,
   -0.300000;-0.000000;0.500000;,
   -0.200000;-0.000000;0.500000;,
   -0.100000;-0.000000;0.500000;,
   0.000000;-0.000000;0.500000;,
   0.100000;-0.000000;0.500000;,
   0.200000;-0.000000;0.500000;,
   0.300000;-0.000000;0.500000;,
   0.400000;-0.000000;0.500000;,
   0.500000;-0.000000;0.500000;,
   -0.500000;-0.000000;0.400000;,
   -0.400000;-0.000000;0.400000;,
   -0.300000;-0.000000;0.400000;,
   -0.200000;-0.000000;0.400000;,
   -0.100000;-0.000000;0.400000;,
   0.000000;-0.000000;0.400000;,
   0.100000;-0.000000;0.400000;,
   0.200000;-0.000000;0.400000;,
   0.300000;-0.000000;0.400000;,
   0.400000;-0.000000;0.400000;,
   0.500000;-0.000000;0.400000;,
   -0.500000;-0.000000;0.300000;,
   -0.400000;-0.000000;0.300000;,
   -0.300000;-0.000000;0.300000;,
   -0.200000;-0.000000;0.300000;,
   -0.100000;-0.000000;0.300000;,
   0.000000;-0.000000;0.300000;,
   0.100000;-0.000000;0.300000;,
   0.200000;-0.000000;0.300000;,
   0.300000;-0.000000;0.300000;,
   0.400000;-0.000000;0.300000;,
   0.500000;-0.000000;0.300000;,
   -0.500000;-0.000000;0.200000;,
   -0.400000;-0.000000;0.200000;,
   -0.300000;-0.000000;0.200000;,
   -0.200000;-0.000000;0.200000;,
   -0.100000;-0.000000;0.200000;,
   0.000000;-0.000000;0.200000;,
   0.100000;-0.000000;0.200000;,
   0.200000;-0.000000;0.200000;,
   0.300000;-0.000000;0.200000;,
   0.400000;-0.000000;0.200000;,
   0.500000;-0.000000;0.200000;,
   -0.500000;-0.000000;0.100000;,
   -0.400000;-0.000000;0.100000;,
   -0.300000;-0.000000;0.100000;,
   -0.200000;-0.000000;0.100000;,
   -0.100000;-0.000000;0.100000;,
   0.000000;-0.000000;0.100000;,
   0.100000;-0.000000;0.100000;,
   0.200000;-0.000000;0.100000;,
   0.300000;-0.000000;0.100000;,
   0.400000;-0.000000;0.100000;,
   0.500000;-0.000000;0.100000;,
   -0.500000;0.000000;-0.000000;,
   -0.400000;0.000000;-0.000000;,
   -0.300000;0.000000;-0.000000;,
   -0.200000;0.000000;-0.000000;,
   -0.100000;0.000000;-0.000000;,
   0.000000;0.000000;-0.000000;,
   0.100000;0.000000;-0.000000;,
   0.200000;0.000000;-0.000000;,
   0.300000;0.000000;-0.000000;,
   0.400000;0.000000;-0.000000;,
   0.500000;0.000000;-0.000000;,
   -0.500000;0.000000;-0.100000;,
   -0.400000;0.000000;-0.100000;,
   -0.300000;0.000000;-0.100000;,
   -0.200000;0.000000;-0.100000;,
   -0.100000;0.000000;-0.100000;,
   0.000000;0.000000;-0.100000;,
   0.100000;0.000000;-0.100000;,
   0.200000;0.000000;-0.100000;,
   0.300000;0.000000;-0.100000;,
   0.400000;0.000000;-0.100000;,
   0.500000;0.000000;-0.100000;,
   -0.500000;0.000000;-0.200000;,
   -0.400000;0.000000;-0.200000;,
   -0.300000;0.000000;-0.200000;,
   -0.200000;0.000000;-0.200000;,
   -0.100000;0.000000;-0.200000;,
   0.000000;0.000000;-0.200000;,
   0.100000;0.000000;-0.200000;,
   0.200000;0.000000;-0.200000;,
   0.300000;0.000000;-0.200000;,
   0.400000;0.000000;-0.200000;,
   0.500000;0.000000;-0.200000;,
   -0.500000;0.000000;-0.300000;,
   -0.400000;0.000000;-0.300000;,
   -0.300000;0.000000;-0.300000;,
   -0.200000;0.000000;-0.300000;,
   -0.100000;0.000000;-0.300000;,
   0.000000;0.000000;-0.300000;,
   0.100000;0.000000;-0.300000;,
   0.200000;0.000000;-0.300000;,
   0.300000;0.000000;-0.300000;,
   0.400000;0.000000;-0.300000;,
   0.500000;0.000000;-0.300000;,
   -0.500000;0.000000;-0.400000;,
   -0.400000;0.000000;-0.400000;,
   -0.300000;0.000000;-0.400000;,
   -0.200000;0.000000;-0.400000;,
   -0.100000;0.000000;-0.400000;,
   0.000000;0.000000;-0.400000;,
   0.100000;0.000000;-0.400000;,
   0.200000;0.000000;-0.400000;,
   0.300000;0.000000;-0.400000;,
   0.400000;0.000000;-0.400000;,
   0.500000;0.000000;-0.400000;,
   -0.500000;0.000000;-0.500000;,
   -0.400000;0.000000;-0.500000;,
   -0.300000;0.000000;-0.500000;,
   -0.200000;0.000000;-0.500000;,
   -0.100000;0.000000;-0.500000;,
   0.000000;0.000000;-0.500000;,
   0.100000;0.000000;-0.500000;,
   0.200000;0.000000;-0.500000;,
   0.300000;0.000000;-0.500000;,
   0.400000;0.000000;-0.500000;,
   0.500000;0.000000;-0.500000;;
   200;
   3;11,1,0;,
   3;11,12,1;,
   3;12,2,1;,
   3;12,13,2;,
   3;13,3,2;,
   3;13,14,3;,
   3;14,4,3;,
   3;14,15,4;,
   3;15,5,4;,
   3;15,16,5;,
   3;16,6,5;,
   3;16,17,6;,
   3;17,7,6;,
   3;17,18,7;,
   3;18,8,7;,
   3;18,19,8;,
   3;19,9,8;,
   3;19,20,9;,
   3;20,10,9;,
   3;20,21,10;,
   3;22,12,11;,
   3;22,23,12;,
   3;23,13,12;,
   3;23,24,13;,
   3;24,14,13;,
   3;24,25,14;,
   3;25,15,14;,
   3;25,26,15;,
   3;26,16,15;,
   3;26,27,16;,
   3;27,17,16;,
   3;27,28,17;,
   3;28,18,17;,
   3;28,29,18;,
   3;29,19,18;,
   3;29,30,19;,
   3;30,20,19;,
   3;30,31,20;,
   3;31,21,20;,
   3;31,32,21;,
   3;33,23,22;,
   3;33,34,23;,
   3;34,24,23;,
   3;34,35,24;,
   3;35,25,24;,
   3;35,36,25;,
   3;36,26,25;,
   3;36,37,26;,
   3;37,27,26;,
   3;37,38,27;,
   3;38,28,27;,
   3;38,39,28;,
   3;39,29,28;,
   3;39,40,29;,
   3;40,30,29;,
   3;40,41,30;,
   3;41,31,30;,
   3;41,42,31;,
   3;42,32,31;,
   3;42,43,32;,
   3;44,34,33;,
   3;44,45,34;,
   3;45,35,34;,
   3;45,46,35;,
   3;46,36,35;,
   3;46,47,36;,
   3;47,37,36;,
   3;47,48,37;,
   3;48,38,37;,
   3;48,49,38;,
   3;49,39,38;,
   3;49,50,39;,
   3;50,40,39;,
   3;50,51,40;,
   3;51,41,40;,
   3;51,52,41;,
   3;52,42,41;,
   3;52,53,42;,
   3;53,43,42;,
   3;53,54,43;,
   3;55,45,44;,
   3;55,56,45;,
   3;56,46,45;,
   3;56,57,46;,
   3;57,47,46;,
   3;57,58,47;,
   3;58,48,47;,
   3;58,59,48;,
   3;59,49,48;,
   3;59,60,49;,
   3;60,50,49;,
   3;60,61,50;,
   3;61,51,50;,
   3;61,62,51;,
   3;62,52,51;,
   3;62,63,52;,
   3;63,53,52;,
   3;63,64,53;,
   3;64,54,53;,
   3;64,65,54;,
   3;66,56,55;,
   3;66,67,56;,
   3;67,57,56;,
   3;67,68,57;,
   3;68,58,57;,
   3;68,69,58;,
   3;69,59,58;,
   3;69,70,59;,
   3;70,60,59;,
   3;70,71,60;,
   3;71,61,60;,
   3;71,72,61;,
   3;72,62,61;,
   3;72,73,62;,
   3;73,63,62;,
   3;73,74,63;,
   3;74,64,63;,
   3;74,75,64;,
   3;75,65,64;,
   3;75,76,65;,
   3;77,67,66;,
   3;77,78,67;,
   3;78,68,67;,
   3;78,79,68;,
   3;79,69,68;,
   3;79,80,69;,
   3;80,70,69;,
   3;80,81,70;,
   3;81,71,70;,
   3;81,82,71;,
   3;82,72,71;,
   3;82,83,72;,
   3;83,73,72;,
   3;83,84,73;,
   3;84,74,73;,
   3;84,85,74;,
   3;85,75,74;,
   3;85,86,75;,
   3;86,76,75;,
   3;86,87,76;,
   3;88,78,77;,
   3;88,89,78;,
   3;89,79,78;,
   3;89,90,79;,
   3;90,80,79;,
   3;90,91,80;,
   3;91,81,80;,
   3;91,92,81;,
   3;92,82,81;,
   3;92,93,82;,
   3;93,83,82;,
   3;93,94,83;,
   3;94,84,83;,
   3;94,95,84;,
   3;95,85,84;,
   3;95,96,85;,
   3;96,86,85;,
   3;96,97,86;,
   3;97,87,86;,
   3;97,98,87;,
   3;99,89,88;,
   3;99,100,89;,
   3;100,90,89;,
   3;100,101,90;,
   3;101,91,90;,
   3;101,102,91;,
   3;102,92,91;,
   3;102,103,92;,
   3;103,93,92;,
   3;103,104,93;,
   3;104,94,93;,
   3;104,105,94;,
   3;105,95,94;,
   3;105,106,95;,
   3;106,96,95;,
   3;106,107,96;,
   3;107,97,96;,
   3;107,108,97;,
   3;108,98,97;,
   3;108,109,98;,
   3;110,100,99;,
   3;110,111,100;,
   3;111,101,100;,
   3;111,112,101;,
   3;112,102,101;,
   3;112,113,102;,
   3;113,103,102;,
   3;113,114,103;,
   3;114,104,103;,
   3;114,115,104;,
   3;115,105,104;,
   3;115,116,105;,
   3;116,106,105;,
   3;116,117,106;,
   3;117,107,106;,
   3;117,118,107;,
   3;118,108,107;,
   3;118,119,108;,
   3;119,109,108;,
   3;119,120,109;;

   MeshNormals {
    1;
    0.000000;1.000000;0.000000;;
    200;
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;;
   }

   MeshTextureCoords {
    121;
    0.000000;1.000000;,
    0.100000;1.000000;,
    0.200000;1.000000;,
    0.300000;1.000000;,
    0.400000;1.000000;,
    0.500000;1.000000;,
    0.600000;1.000000;,
    0.700000;1.000000;,
    0.800000;1.000000;,
    0.900000;1.000000;,
    1.000000;1.000000;,
    0.000000;0.900000;,
    0.100000;0.900000;,
    0.200000;0.900000;,
    0.300000;0.900000;,
    0.400000;0.900000;,
    0.500000;0.900000;,
    0.600000;0.900000;,
    0.700000;0.900000;,
    0.800000;0.900000;,
    0.900000;0.900000;,
    1.000000;0.900000;,
    0.000000;0.800000;,
    0.100000;0.800000;,
    0.200000;0.800000;,
    0.300000;0.800000;,
    0.400000;0.800000;,
    0.500000;0.800000;,
    0.600000;0.800000;,
    0.700000;0.800000;,
    0.800000;0.800000;,
    0.900000;0.800000;,
    1.000000;0.800000;,
    0.000000;0.700000;,
    0.100000;0.700000;,
    0.200000;0.700000;,
    0.300000;0.700000;,
    0.400000;0.700000;,
    0.500000;0.700000;,
    0.600000;0.700000;,
    0.700000;0.700000;,
    0.800000;0.700000;,
    0.900000;0.700000;,
    1.000000;0.700000;,
    0.000000;0.600000;,
    0.100000;0.600000;,
    0.200000;0.600000;,
    0.300000;0.600000;,
    0.400000;0.600000;,
    0.500000;0.600000;,
    0.600000;0.600000;,
    0.700000;0.600000;,
    0.800000;0.600000;,
    0.900000;0.600000;,
    1.000000;0.600000;,
    0.000000;0.500000;,
    0.100000;0.500000;,
    0.200000;0.500000;,
    0.300000;0.500000;,
    0.400000;0.500000;,
    0.500000;0.500000;,
    0.600000;0.500000;,
    0.700000;0.500000;,
    0.800000;0.500000;,
    0.900000;0.500000;,
    1.000000;0.500000;,
    0.000000;0.400000;,
    0.100000;0.400000;,
    0.200000;0.400000;,
    0.300000;0.400000;,
    0.400000;0.400000;,
    0.500000;0.400000;,
    0.600000;0.400000;,
    0.700000;0.400000;,
    0.800000;0.400000;,
    0.900000;0.400000;,
    1.000000;0.400000;,
    0.000000;0.300000;,
    0.100000;0.300000;,
    0.200000;0.300000;,
    0.300000;0.300000;,
    0.400000;0.300000;,
    0.500000;0.300000;,
    0.600000;0.300000;,
    0.700000;0.300000;,
    0.800000;0.300000;,
    0.900000;0.300000;,
    1.000000;0.300000;,
    0.000000;0.200000;,
    0.100000;0.200000;,
    0.200000;0.200000;,
    0.300000;0.200000;,
    0.400000;0.200000;,
    0.500000;0.200000;,
    0.600000;0.200000;,
    0.700000;0.200000;,
    0.800000;0.200000;,
    0.900000;0.200000;,
    1.000000;0.200000;,
    0.000000;0.100000;,
    0.100000;0.100000;,
    0.200000;0.100000;,
    0.300000;0.100000;,
    0.400000;0.100000;,
    0.500000;0.100000;,
    0.600000;0.100000;,
    0.700000;0.100000;,
    0.800000;0.100000;,
    0.900000;0.100000;,
    1.000000;0.100000;,
    0.000000;0.000000;,
    0.100000;0.000000;,
    0.200000;0.000000;,
    0.300000;0.000000;,
    0.400000;0.000000;,
    0.500000;0.000000;,
    0.600000;0.000000;,
    0.700000;0.000000;,
    0.800000;0.000000;,
    0.900000;0.000000;,
    1.000000;0.000000;;
   }

   MeshMaterialList {
    1;
    200;
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0;

    Material blinn1SG {
     0.800000;0.800000;0.800000;1.000000;;
     0.500000;
     0.500000;0.500000;0.500000;;
     0.000000;0.000000;0.000000;;

     TextureFilename {
      "C:\\Documents and Settings\\msheffield\\Desktop\\NV40_SDK\\Blood\\cobble_color.tga";
     }
    }
   }

   VertexDuplicationIndices {
    121;
    121;
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
    34,
    35,
    36,
    37,
    38,
    39,
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,
    62,
    63,
    64,
    65,
    66,
    67,
    68,
    69,
    70,
    71,
    72,
    73,
    74,
    75,
    76,
    77,
    78,
    79,
    80,
    81,
    82,
    83,
    84,
    85,
    86,
    87,
    88,
    89,
    90,
    91,
    92,
    93,
    94,
    95,
    96,
    97,
    98,
    99,
    100,
    101,
    102,
    103,
    104,
    105,
    106,
    107,
    108,
    109,
    110,
    111,
    112,
    113,
    114,
    115,
    116,
    117,
    118,
    119,
    120;
   }
  }
 }
}