// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

#include <cstdio>
#include <string>
#include <vector>

#include <OpenImageIO/sysutil.h>

#include <OSL/genclosure.h>
#include <OSL/oslclosure.h>
#include <OSL/oslconfig.h>

#include "oslexec_pvt.h"



OSL_NAMESPACE_BEGIN

const ustring Labels::NONE       = ustring(NULL);
const ustring Labels::CAMERA     = ustring("C");
const ustring Labels::LIGHT      = ustring("L");
const ustring Labels::BACKGROUND = ustring("B");
const ustring Labels::VOLUME     = ustring("V");
const ustring Labels::OBJECT     = ustring("O");
const ustring Labels::TRANSMIT   = ustring("T");
const ustring Labels::REFLECT    = ustring("R");
const ustring Labels::DIFFUSE    = ustring("D");
const ustring Labels::GLOSSY     = ustring("G");
const ustring Labels::SINGULAR   = ustring("S");
const ustring Labels::STRAIGHT   = ustring("s");
const ustring Labels::STOP       = ustring("__stop__");



namespace pvt {



static void
print_component_value(std::ostream& out, ShadingSystemImpl* ss, TypeDesc type,
                      const void* data, bool treat_ustrings_as_hash)

{
    if (type == TypeInt)
        out << *(int*)data;
    else if (type == TypeFloat)
        out << *(float*)data;
    else if (type == TypeColor)
        out << "(" << ((Color3*)data)->x << ", " << ((Color3*)data)->y << ", "
            << ((Color3*)data)->z << ")";
    else if (type == TypeVector)
        out << "(" << ((Vec3*)data)->x << ", " << ((Vec3*)data)->y << ", "
            << ((Vec3*)data)->z << ")";
    else if (type == TypeString) {
        if (treat_ustrings_as_hash == true) {
            out << "\"" << *((ustringhash*)data) << "\"";
        } else {
            out << "\"" << *((ustring*)data) << "\"";
        }
    } else if (type == TypeDesc::PTR)  // this only happens for closures
        print_closure(out, *(const ClosureColor**)data, ss,
                      treat_ustrings_as_hash);
}



static void
print_component(std::ostream& out, const ClosureComponent* comp,
                ShadingSystemImpl* ss, const Color3& weight,
                bool& treat_ustrings_as_hash)
{
    const ClosureRegistry::ClosureEntry* clentry = ss->find_closure(comp->id);
    OSL_ASSERT(clentry);
    out << "(" << weight.x * comp->w.x << ", " << weight.y * comp->w.y << ", "
        << weight.z * comp->w.z << ") * ";
    out << clentry->name.c_str() << " (";
    for (int i = 0, nparams = clentry->params.size() - 1; i < nparams; ++i) {
        if (i)
            out << ", ";
        const ClosureParam& param = clentry->params[i];
        if (param.key != 0)
            out << "\"" << param.key << "\", ";
        if (param.type.numelements() > 1)
            out << "[";
        for (size_t j = 0; j < param.type.numelements(); ++j) {
            if (j)
                out << ", ";
            print_component_value(out, ss, param.type.elementtype(),
                                  (const char*)comp->data() + param.offset
                                      + param.type.elementsize() * j,
                                  treat_ustrings_as_hash);
        }
        if (clentry->params[i].type.numelements() > 1)
            out << "]";
    }
    out << ")";
}



static void
print_closure(std::ostream& out, const ClosureColor* closure,
              ShadingSystemImpl* ss, const Color3& w, bool& first,
              bool& treat_ustrings_as_hash)
{
    if (closure == NULL)
        return;

    switch (closure->id) {
    case ClosureColor::MUL:
        print_closure(out, closure->as_mul()->closure, ss,
                      closure->as_mul()->weight * w, first,
                      treat_ustrings_as_hash);
        break;
    case ClosureColor::ADD:
        print_closure(out, closure->as_add()->closureA, ss, w, first,
                      treat_ustrings_as_hash);
        print_closure(out, closure->as_add()->closureB, ss, w, first,
                      treat_ustrings_as_hash);
        break;
    default:
        if (!first)
            out << "\n\t+ ";
        print_component(out, closure->as_comp(), ss, w, treat_ustrings_as_hash);
        first = false;
        break;
    }
}



void
print_closure(std::ostream& out, const ClosureColor* closure,
              ShadingSystemImpl* ss, bool treat_ustrings_as_hash)
{
    bool first = true;
    print_closure(out, closure, ss, Color3(1, 1, 1), first,
                  treat_ustrings_as_hash);
}



}  // namespace pvt



OSL_NAMESPACE_END
