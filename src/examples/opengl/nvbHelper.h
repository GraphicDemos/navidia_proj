#ifndef __NVB_HELPER_H__
#define __NVB_HELPER_H__

#include <nv_nvb/nv_nvb.h>
#include <nv_dds/nv_dds.h>
#include <string>

class nvbHelper
{
    public:
        void Load(std::string path, std::string filename)
        {        
            // load model
            std::string modelFile = path + "\\" + filename;
            NVBLoad(modelFile.c_str(), &m_scene);
            
            // load textures
            m_gl_textures = new GLuint[m_scene.num_textures];
            loadTextures(path);
        }

        void Draw()
        {
            // draw opaque models first...
            drawModels(true);

            // draw transparent models next...
            // note: we don't sort even if we should!
            drawModels(false);
        }

        inline const nv_scene &GetScene()
        {
            return m_scene;
        }

        inline GLuint GetTexture(int index)
        {
            assert(index >= 0 && index < m_num_textures);

            return m_gl_textures[index];
        }

    private:
        void loadTextures(std::string modelPath)
        {
            m_num_textures = m_scene.num_textures;
            glGenTextures(m_num_textures, m_gl_textures);

            for (unsigned int i = 0; i < m_scene.num_textures; ++i)
            {
                std::string textureFilename = modelPath + "\\" + m_scene.textures[i].name;

                nv_dds::CDDSImage image;
                if (!image.load(textureFilename))
                {
                    cout << "Unable to load " << textureFilename << endl;
                    continue;
                }

                if (image.get_num_mipmaps() != 0)
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                glBindTexture(GL_TEXTURE_2D, m_gl_textures[i]);
                image.upload_texture2D();
            }
        }

        void drawModels(bool opaque)
        {
            // simple tetxure state caching
            bool texturing = false; 

            if (opaque == false)
            {
                // enable transparency blending...
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            
            for (unsigned int i = 0; i < m_scene.num_nodes; ++i)
            {
                nv_node *node = m_scene.nodes[i];

                if (node->get_type() == nv_node::GEOMETRY)
                {
                    nv_model *model = reinterpret_cast<nv_model*>(node);

                    if (model->num_meshes == 0)
                        continue;

                    // lets push the object hierarchy...
                    glPushMatrix();
                    glMultMatrixf(model->xform.mat_array);

                    for (unsigned int j = 0; j < model->num_meshes; ++j)
                    {
                        nv_mesh &mesh = model->meshes[j];
                        if (mesh.material_id == NV_BAD_IDX)
                            break;

                        if (mesh.skin)
                            break;

                        nv_material &mat = m_scene.materials[mesh.material_id];
                        glNormalPointer(GL_FLOAT, 0, mesh.normals);

                        if (mesh.num_texcoord_sets)
                            glTexCoordPointer(2, GL_FLOAT, 0, mesh.texcoord_sets[0].texcoords);

                        glVertexPointer(3, GL_FLOAT, 0, mesh.vertices);
            
                        if ((opaque == true && mat.transparent == nv_one) ||
                            (opaque == false && mat.transparent < nv_one))
                        {
                            // look for a diffuse texture...
                            nv_idx diffuse_id = NV_BAD_IDX;
                            for (unsigned int k = 0; k < mat.num_textures && diffuse_id == NV_BAD_IDX; ++k)
                            {
                                if (m_scene.textures[mat.textures[k]].type == nv_texture::DIFFUSE)
                                    diffuse_id = mat.textures[k];
                            }

                            if (diffuse_id != NV_BAD_IDX)
                            {
                                // we found a diffuse texture, we can enable 
                                // texturing if it hasn't been already set.
                                if (texturing == false)
                                {
                                    glEnable(GL_TEXTURE_2D);
                                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                                    texturing = true;
                                }
                    
                                glBindTexture(GL_TEXTURE_2D, m_gl_textures[diffuse_id]);
                            }
                            else
                            {
                                texturing = false;
                                glDisable(GL_TEXTURE_2D);
                                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                            }

                            glEnableClientState(GL_NORMAL_ARRAY);
                            glEnableClientState(GL_VERTEX_ARRAY);

                            glDrawElements(GL_TRIANGLES, mesh.num_faces * 3, GL_UNSIGNED_INT, mesh.faces_idx);

                            glDisableClientState(GL_NORMAL_ARRAY);
                            glDisableClientState(GL_VERTEX_ARRAY);
                        }
                    }

                    glPopMatrix();
                }
            }

            if (opaque == false)
                glDisable(GL_BLEND);

            if (texturing)
            {
                glDisable(GL_TEXTURE_2D);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        }

        nv_scene m_scene;
        GLuint *m_gl_textures;
        int m_num_textures;
};

#endif
