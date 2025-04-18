// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

#pragma once

#include <map>
#include <memory>
#include <unordered_map>

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/thread.h>
#include <OpenImageIO/ustring.h>

#include <OSL/oslexec.h>
#include <OSL/rendererservices.h>

OSL_NAMESPACE_BEGIN



class OSLToyRenderer final : public RendererServices {
public:
    // Just use 4x4 matrix for transformations
    typedef Matrix44 Transformation;

    OSLToyRenderer();
    ~OSLToyRenderer() {}

    ShadingSystem* shadingsys() const { return m_shadingsys; }

    // Super simple camera and display parameters.  Many options not
    // available, no motion blur, etc.
    void camera_params(const Matrix44& world_to_camera, ustringhash projection,
                       float hfov, float hither, float yon, int xres, int yres);

    void set_resolution(int x, int y)
    {
        m_xres = x;
        m_yres = y;
    }

    void set_time(float t) { m_shaderglobals_template.time = t; }

    void set_mouse(int x, int y)
    {
        m_mouse_x = x;
        m_mouse_y = y;
    }

    void set_shadergroup(ShaderGroupRef g)
    {
        OIIO::spin_lock lock(m_mutex);
        m_group = g;
    }

    ShaderGroupRef shadergroup()
    {
        OIIO::spin_lock lock(m_mutex);
        return m_group;
    }

    OIIO::ImageBuf& framebuffer() { return m_framebuffer; }

    void render_image();

    // vvv Methods necessary to be a RendererServices
    virtual int supports(string_view feature) const;
    virtual bool get_matrix(ShaderGlobals* sg, Matrix44& result,
                            TransformationPtr xform, float time);
    virtual bool get_matrix(ShaderGlobals* sg, Matrix44& result,
                            ustringhash from, float time);
    virtual bool get_matrix(ShaderGlobals* sg, Matrix44& result,
                            TransformationPtr xform);
    virtual bool get_matrix(ShaderGlobals* sg, Matrix44& result,
                            ustringhash from);
    virtual bool get_inverse_matrix(ShaderGlobals* sg, Matrix44& result,
                                    ustringhash to, float time);

    void name_transform(const char* name, const Transformation& xform);

    virtual bool get_array_attribute(ShaderGlobals* sg, bool derivatives,
                                     ustringhash object, TypeDesc type,
                                     ustringhash name, int index, void* val);
    virtual bool get_attribute(ShaderGlobals* sg, bool derivatives,
                               ustringhash object, TypeDesc type,
                               ustringhash name, void* val);
    virtual bool get_userdata(bool derivatives, ustringhash name, TypeDesc type,
                              ShaderGlobals* sg, void* val);

private:
    OIIO::spin_mutex m_mutex;
    ShadingSystem* m_shadingsys;
    ShaderGroupRef m_group;
    ShaderGlobals m_shaderglobals_template;
    OIIO::ImageBuf m_framebuffer;

    // Camera parameters
    Matrix44 m_world_to_camera;
    ustringhash m_projection;
    float m_fov, m_pixelaspect, m_hither, m_yon;
    float m_shutter[2];
    float m_screen_window[4];
    int m_xres, m_yres;
    int m_mouse_x = -1, m_mouse_y = -1;

    // Named transforms
    typedef std::map<ustringhash, std::shared_ptr<Transformation>> TransformMap;
    TransformMap m_named_xforms;

    // Attribute and userdata retrieval -- for fast dispatch, use a hash
    // table to map attribute names to functions that retrieve them. We
    // imagine this to be fairly quick, but for a performance-critical
    // renderer, we would encourage benchmarking various methods and
    // alternate data structures.
    typedef bool (OSLToyRenderer::*AttrGetter)(ShaderGlobals* sg, bool derivs,
                                               ustringhash object,
                                               TypeDesc type, ustringhash name,
                                               void* val);
    typedef std::unordered_map<ustringhash, AttrGetter> AttrGetterMap;
    AttrGetterMap m_attr_getters;

    // Attribute getters
    bool get_osl_version(ShaderGlobals* sg, bool derivs, ustringhash object,
                         TypeDesc type, ustringhash name, void* val);
    bool get_camera_resolution(ShaderGlobals* sg, bool derivs,
                               ustringhash object, TypeDesc type,
                               ustringhash name, void* val);
    bool get_camera_projection(ShaderGlobals* sg, bool derivs,
                               ustringhash object, TypeDesc type,
                               ustringhash name, void* val);
    bool get_camera_fov(ShaderGlobals* sg, bool derivs, ustringhash object,
                        TypeDesc type, ustringhash name, void* val);
    bool get_camera_pixelaspect(ShaderGlobals* sg, bool derivs,
                                ustringhash object, TypeDesc type,
                                ustringhash name, void* val);
    bool get_camera_clip(ShaderGlobals* sg, bool derivs, ustringhash object,
                         TypeDesc type, ustringhash name, void* val);
    bool get_camera_clip_near(ShaderGlobals* sg, bool derivs,
                              ustringhash object, TypeDesc type,
                              ustringhash name, void* val);
    bool get_camera_clip_far(ShaderGlobals* sg, bool derivs, ustringhash object,
                             TypeDesc type, ustringhash name, void* val);
    bool get_camera_shutter(ShaderGlobals* sg, bool derivs, ustringhash object,
                            TypeDesc type, ustringhash name, void* val);
    bool get_camera_shutter_open(ShaderGlobals* sg, bool derivs,
                                 ustringhash object, TypeDesc type,
                                 ustringhash name, void* val);
    bool get_camera_shutter_close(ShaderGlobals* sg, bool derivs,
                                  ustringhash object, TypeDesc type,
                                  ustringhash name, void* val);
    bool get_camera_screen_window(ShaderGlobals* sg, bool derivs,
                                  ustringhash object, TypeDesc type,
                                  ustringhash name, void* val);
};

OSL_NAMESPACE_END
