// Auto generated, do not edit.
#ifndef __fixed_ps_shaders_h_
#define __fixed_ps_shaders_h_

#ifndef SHADER_SELECTOR_PARAMS_fixed_ps
#define SHADER_SELECTOR_PARAMS_fixed_ps
#endif
#ifndef SHADER_SELECTOR_INIT_fixed_ps
#define SHADER_SELECTOR_INIT_fixed_ps
#endif

// Shader enum
enum e_fixed_ps {
  e_fixed_ps__default = 0,
  e_fixed_ps_alphatest = 1,
  e_fixed_ps_base = 2,
  e_fixed_ps_base_alphatest = 3,
  e_fixed_ps_base_distancefield = 4,
  e_fixed_ps_base_opacity_alphatest = 5,
  e_fixed_ps_base_opacity_translucent = 6,
  e_fixed_ps_base_translucent = 7,
  e_fixed_ps_decal = 8,
  e_fixed_ps_decal_alphatest = 9,
  e_fixed_ps_decal_base = 10,
  e_fixed_ps_decal_base_alphatest = 11,
  e_fixed_ps_decal_base_distancefield = 12,
  e_fixed_ps_decal_base_opacity_alphatest = 13,
  e_fixed_ps_decal_base_opacity_translucent = 14,
  e_fixed_ps_decal_base_translucent = 15,
  e_fixed_ps_decal_distancefield = 16,
  e_fixed_ps_decal_opacity_alphatest = 17,
  e_fixed_ps_decal_opacity_translucent = 18,
  e_fixed_ps_decal_translucent = 19,
  e_fixed_ps_distancefield = 20,
  e_fixed_ps_opacity_alphatest = 21,
  e_fixed_ps_opacity_translucent = 22,
  e_fixed_ps_translucent = 23,
  e_fixed_ps_last = 24
};

// Shader names
static const char* fn_fixed_ps[e_fixed_ps_last] = {
  "fixed_ps",
  "fixed_ps_alphatest",
  "fixed_ps_base",
  "fixed_ps_base_alphatest",
  "fixed_ps_base_distancefield",
  "fixed_ps_base_opacity_alphatest",
  "fixed_ps_base_opacity_translucent",
  "fixed_ps_base_translucent",
  "fixed_ps_decal",
  "fixed_ps_decal_alphatest",
  "fixed_ps_decal_base",
  "fixed_ps_decal_base_alphatest",
  "fixed_ps_decal_base_distancefield",
  "fixed_ps_decal_base_opacity_alphatest",
  "fixed_ps_decal_base_opacity_translucent",
  "fixed_ps_decal_base_translucent",
  "fixed_ps_decal_distancefield",
  "fixed_ps_decal_opacity_alphatest",
  "fixed_ps_decal_opacity_translucent",
  "fixed_ps_decal_translucent",
  "fixed_ps_distancefield",
  "fixed_ps_opacity_alphatest",
  "fixed_ps_opacity_translucent",
  "fixed_ps_translucent",
};

// Shader Selector
static tU32 select_shader_fixed_ps(const sMaterialDesc& matDesc SHADER_SELECTOR_PARAMS_fixed_ps) {
  const tMaterialFlags matFlags = matDesc.mFlags;
  SHADER_SELECTOR_INIT_fixed_ps;
  if (SHADER_MATERIAL_HAS_FLAG(Decal)) {
    if (SHADER_MATERIAL_HAS_CHANNEL(Base)) {
      if (SHADER_MATERIAL_HAS_CHANNEL(Opacity)) {
        if (SHADER_MATERIAL_HAS_FLAG(Translucent)) {
          return e_fixed_ps_decal_base_opacity_translucent;
        }

        if (SHADER_MATERIAL_HAS_FLAG(Transparent)) {
          return e_fixed_ps_decal_base_opacity_alphatest;
        }

      }

      if (SHADER_MATERIAL_HAS_FLAG(DistanceField)) {
        return e_fixed_ps_decal_base_distancefield;
      }

      if (SHADER_MATERIAL_HAS_FLAG(Translucent)) {
        return e_fixed_ps_decal_base_translucent;
      }

      if (SHADER_MATERIAL_HAS_FLAG(Transparent)) {
        return e_fixed_ps_decal_base_alphatest;
      }

      return e_fixed_ps_decal_base;
    }

    if (SHADER_MATERIAL_HAS_CHANNEL(Opacity)) {
      if (SHADER_MATERIAL_HAS_FLAG(Translucent)) {
        return e_fixed_ps_decal_opacity_translucent;
      }

      if (SHADER_MATERIAL_HAS_FLAG(Transparent)) {
        return e_fixed_ps_decal_opacity_alphatest;
      }

    }

    if (SHADER_MATERIAL_HAS_FLAG(DistanceField)) {
      return e_fixed_ps_decal_distancefield;
    }

    if (SHADER_MATERIAL_HAS_FLAG(Translucent)) {
      return e_fixed_ps_decal_translucent;
    }

    if (SHADER_MATERIAL_HAS_FLAG(Transparent)) {
      return e_fixed_ps_decal_alphatest;
    }

    return e_fixed_ps_decal;
  }

  if (SHADER_MATERIAL_HAS_CHANNEL(Base)) {
    if (SHADER_MATERIAL_HAS_CHANNEL(Opacity)) {
      if (SHADER_MATERIAL_HAS_FLAG(Translucent)) {
        return e_fixed_ps_base_opacity_translucent;
      }

      if (SHADER_MATERIAL_HAS_FLAG(Transparent)) {
        return e_fixed_ps_base_opacity_alphatest;
      }

    }

    if (SHADER_MATERIAL_HAS_FLAG(DistanceField)) {
      return e_fixed_ps_base_distancefield;
    }

    if (SHADER_MATERIAL_HAS_FLAG(Translucent)) {
      return e_fixed_ps_base_translucent;
    }

    if (SHADER_MATERIAL_HAS_FLAG(Transparent)) {
      return e_fixed_ps_base_alphatest;
    }

    return e_fixed_ps_base;
  }

  if (SHADER_MATERIAL_HAS_CHANNEL(Opacity)) {
    if (SHADER_MATERIAL_HAS_FLAG(Translucent)) {
      return e_fixed_ps_opacity_translucent;
    }

    if (SHADER_MATERIAL_HAS_FLAG(Transparent)) {
      return e_fixed_ps_opacity_alphatest;
    }

  }

  if (SHADER_MATERIAL_HAS_FLAG(DistanceField)) {
    return e_fixed_ps_distancefield;
  }

  if (SHADER_MATERIAL_HAS_FLAG(Translucent)) {
    return e_fixed_ps_translucent;
  }

  if (SHADER_MATERIAL_HAS_FLAG(Transparent)) {
    return e_fixed_ps_alphatest;
  }

  return e_fixed_ps__default;
}
#endif // __fixed_ps_shaders_h_
