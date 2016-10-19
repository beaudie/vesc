#ifndef LIBANGLE_INTEL_DRIVER_H_
#define LIBANGLE_INTEL_DRIVER_H_

#include "angletypes.h"

namespace rx
{
namespace Vendor_Intel
{
// Referenced from: https://cgit.freedesktop.org/vaapi/intel-driver/tree/src/i965_device_info.c
struct intel_device_info
{
    int gen;
    int gt;

    unsigned int is_g4x : 1;        /* gen4 */
    unsigned int is_ivybridge : 1;  /* gen7 */
    unsigned int is_baytrail : 1;   /* gen7 */
    unsigned int is_haswell : 1;    /* gen7 */
    unsigned int is_cherryview : 1; /* gen8 */
    unsigned int is_skylake : 1;    /* gen9 */
    unsigned int is_broxton : 1;    /* gen9 */
    unsigned int is_kabylake : 1;   /* gen9p5 */
};

static const struct intel_device_info g4x_device_info = {
    4,  // .gen = 4,
    0,  //

    1,  // .is_g4x = 1,
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info ilk_device_info = {
    5,  // .gen = 5,
    0,  //

    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info snb_gt1_device_info = {
    6,  // .gen = 6,
    1,  // .gt = 1,

    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info snb_gt2_device_info = {
    6,  // .gen = 6,
    2,  // .gt = 2,

    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info ivb_gt1_device_info = {
    7,  // .gen = 7,
    1,  // .gt = 1,

    0,  //
    1,  // .is_ivybridge = 1,
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info ivb_gt2_device_info = {
    7,  // .gen = 7,
    2,  // .gt = 2,

    0,  //
    1,  // .is_ivybridge = 1,
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info byt_device_info = {
    7,  // .gen = 7,
    1,  // .gt = 1,

    0,  //
    1,  // .is_ivybridge = 1,
    1,  // .is_baytrail	= 1,
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info hsw_gt1_device_info = {
    7,  // .gen = 7,
    1,  // .gt = 1,

    0,  //
    0,  //
    0,  //
    1,  // .is_haswell = 1,
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info hsw_gt2_device_info = {
    7,  // .gen = 7,
    2,  // .gt = 2,

    0,  //
    0,  //
    0,  //
    1,  // .is_haswell = 1,
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info hsw_gt3_device_info = {
    7,  // .gen = 7,
    3,  // .gt = 3,

    0,  //
    0,  //
    0,  //
    1,  // .is_haswell = 1,
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info bdw_device_info = {
    8,  // .gen = 8,
    0,  //

    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info chv_device_info = {
    8,  // .gen = 8,
    0,  //

    0,  //
    0,  //
    0,  //
    0,  //
    1,  // .is_cherryview = 1,
    0,  //
    0,  //
    0,  //
};

static const struct intel_device_info skl_device_info = {
    9,  // .gen = 9,
    0,  //

    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    1,  // .is_skylake = 1,
    0,  //
    0,  //
};

static const struct intel_device_info bxt_device_info = {
    9,  // .gen = 9,
    0,  //

    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    1,  // .is_broxton = 1,
    0,  //
};

static const struct intel_device_info kbl_device_info = {
    9,  // .gen = 9,
    0,  //

    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    0,  //
    1,  // .is_kabylake = 1,
};

#define IS_G4X(device_info) (device_info->is_g4x)

#define IS_IRONLAKE(device_info) (device_info->gen == 5)

#define IS_GEN6(device_info) (device_info->gen == 6)

#define IS_HASWELL(device_info) (device_info->is_haswell)
#define IS_GEN7(device_info) (device_info->gen == 7)

#define IS_CHERRYVIEW(device_info) (device_info->is_cherryview)
#define IS_GEN8(device_info) (device_info->gen == 8)

#define IS_GEN9(device_info) (device_info->gen == 9)

#define IS_SKL(device_info) (device_info->is_skylake)

#define IS_BXT(device_info) (device_info->is_broxton)

#define IS_KBL(device_info) (device_info->is_kabylake)

bool IsHaswell(uint32_t devid);
bool IsBroadwell(uint32_t devid);
bool IsSkylake(uint32_t devid);

}  // namespace Vendor_Intel
}  // namespace rx
#endif