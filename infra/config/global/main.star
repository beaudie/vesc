#!/usr/bin/env lucicfg

lucicfg.config(fail_on_warnings = True)

luci.project(
    name = "angle",
    buildbucket = "cr-buildbucket.appspot.com",
    logdog = "luci-logdog.appspot.com",
    milo = "luci-milo.appspot.com",
    notify = "luci-notify.appspot.com",
    scheduler = "luci-scheduler.appspot.com",
    swarming = "chromium-swarm.appspot.com",
    acls = [
        acl.entry(
            roles = [
                acl.PROJECT_CONFIGS_READER,
                acl.LOGDOG_READER,
                acl.BUILDBUCKET_READER,
                acl.SCHEDULER_READER,
            ],
            groups = "all",
        ),
        acl.entry(
            roles = [
                acl.SCHEDULER_OWNER,
            ],
            groups = "project-angle-admins"
        )
    ]
)

# The category for an os: a more generic grouping than specific OS versions that
# can be used for computing defaults
os_category = struct(
    ANDROID = "Android",
    LINUX = "Linux",
    MAC = "Mac",
    WINDOWS = "Windows",
)

# The os constants to be used for the os parameter of the builder function
# The *_DEFAULT members enable distinguishing between a use that runs the
# "current" version of the OS and a use that runs against a specific version
# that happens to be the "current" version
def os_enum(dimension, category, console_name):
    return struct(dimension = dimension, category = category, console_name = console_name)

os = struct(
    ANDROID = os_enum("Android", os_category.ANDROID, "android"),
    LINUX_DEFAULT = os_enum("Ubuntu-16.04", os_category.LINUX, "linux"),
    MAC_ANY = os_enum("Mac", os_category.MAC, "mac"),
    WINDOWS_ANY = os_enum("Windows", os_category.WINDOWS, "win"),
)

# Recipes

_RECIPE_NAME_PREFIX = "recipe:"
_DEFAULT_BUILDERLESS_OS_CATEGORIES = [os_category.LINUX, os_category.WINDOWS]
_GOMA_RBE_PROD = {
    "server_host": "goma.chromium.org",
    "rpc_extra_params": "?prod",
}

def _recipe_for_package(cipd_package):
    def recipe(*, name, cipd_version = None, recipe = None, use_bbagent = False):
        # Force the caller to put the recipe prefix rather than adding it
        # programatically to make the string greppable
        if not name.startswith(_RECIPE_NAME_PREFIX):
            fail("Recipe name {!r} does not start with {!r}"
                .format(name, _RECIPE_NAME_PREFIX))
        if recipe == None:
            recipe = name[len(_RECIPE_NAME_PREFIX):]
        return luci.recipe(
            name = name,
            cipd_package = cipd_package,
            cipd_version = cipd_version,
            recipe = recipe,
            use_bbagent = use_bbagent,
        )

    return recipe

build_recipe = _recipe_for_package(
    "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
)

build_recipe(
    name = "recipe:angle",
)

build_recipe(
    name = "recipe:run_presubmit"
)

def get_os_from_name(name):
    if name.startswith("linux"):
        return os.LINUX_DEFAULT
    if name.startswith("win"):
        return os.WINDOWS_ANY
    if name.startswith("mac"):
        return os.MAC_ANY
    return os.ANDROID

def angle_mirrored_builder(name, clang, debug, cpu, uwp, trace_tests, views):
    properties = {
        "debug": debug,
        "target_cpu": cpu,
    }
    os = get_os_from_name(name)
    dimensions = {}
    dimensions["os"] = os.dimension

    goma_props = {}
    goma_props.update(_GOMA_RBE_PROD)

    if os.category in _DEFAULT_BUILDERLESS_OS_CATEGORIES:
        dimensions["builderless"] = "1"
        goma_props["enable_ats"] = True

    properties["$build/goma"] = goma_props

    caches = [swarming.cache(name = "win_toolchain", path = "win_toolchain")]

    if os.category == os_category.MAC:
        # Cache for mac_toolchain tool and XCode.app
        caches += [swarming.cache(name = "osx_sdk", path = "osx_sdk")]
        properties["$depot_tools/osx_sdk"] = {
            "sdk_version": "11b52"
        }

    if not clang:
        properties["clang"] = False

    if uwp:
        properties["uwp"] = True

    if trace_tests:
        properties["trace_tests"] = True

    luci.builder(
        name = "ci/" + name,
        bucket = "ci",
        triggered_by = ["master-poller"],
        executable = "recipe:angle",
        service_account = "angle-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
        properties = properties,
        dimensions = dimensions,
        caches = caches,
        build_numbers = True,
    )
    luci.builder(
        name = "try/" + name,
        bucket = "try",
        executable = "recipe:angle",
        service_account = "angle-try-builder@chops-service-accounts.iam.gserviceaccount.com",
        properties = properties,
        dimensions = dimensions,
        caches = caches,
        build_numbers = True,
    )

    config = "clang"
    if not clang:
        if os.category == os_category.WINDOWS:
            config = "msvc"
        else:
            config = "gcc"
    elif trace_tests:
        config = "trace"
    elif uwp:
        config = "uwp"

    short_name = "dbg" if debug else "rel"

    views += [
        luci.console_view_entry(
            builder = "ci/" + name,
            category = os.console_name + "|" + config + "|" + cpu,
            short_name = short_name,
        )
    ]

luci.bucket(
    name = "ci",
    acls = [
        acl.entry(
            acl.SCHEDULER_TRIGGERER,
            users = [
                "angle-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
                "luci-scheduler@appspot.gserviceaccount.com",
            ],
        ),
    ],
)

luci.bucket(
    name = "try",
    acls = [
        acl.entry(
            acl.BUILDBUCKET_TRIGGERER,
            groups = [
                "project-angle-tryjob-access",
                "service-account-cq",
            ],
        ),
    ],
)

luci.builder(
    name = "presubmit",
    bucket = "try",
    executable = "recipe:run_presubmit",
    service_account = "angle-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    build_numbers = True,
    dimensions = {
        "os": os.LINUX_DEFAULT.dimension,
    },
    properties = {
        "repo_name": "angle",
        "runhooks": True,
    },
)

luci.gitiles_poller(
    name = "master-poller",
    bucket = "ci",
    repo = "https://chromium.googlesource.com/angle/angle",
    refs = [
        "refs/heads/master",
    ],
    schedule = "with 10s interval",
)

# name, clang, debug, cpu, uwp, trace_tests
views = []

angle_mirrored_builder("linux-clang-dbg", True, True, "x64", False, False, views)
angle_mirrored_builder("linux-clang-rel", True, False, "x64", False, False, views)
angle_mirrored_builder("linux-gcc-dbg", False, True, "x64", False, False, views)
angle_mirrored_builder("linux-gcc-rel", False, False, "x64", False, False, views)
angle_mirrored_builder("linux-trace-rel", True, False, "x64", False, True, views)
angle_mirrored_builder("mac-dbg", True, True, "x64", False, False, views)
angle_mirrored_builder("mac-rel", True, False, "x64", False, False, views)
angle_mirrored_builder("win-clang-x86-dbg", True, True, "x86", False, False, views)
angle_mirrored_builder("win-clang-x86-rel", True, False, "x86", False, False, views)
angle_mirrored_builder("win-clang-x64-dbg", True, True, "x64", False, False, views)
angle_mirrored_builder("win-clang-x64-rel", True, False, "x64", False, False, views)
angle_mirrored_builder("win-msvc-x86-dbg", False, True, "x86", False, False, views)
angle_mirrored_builder("win-msvc-x86-rel", False, False, "x86", False, False, views)
angle_mirrored_builder("win-msvc-x64-dbg", False, True, "x64", False, False, views)
angle_mirrored_builder("win-msvc-x64-rel", False, False, "x64", False, False, views)
angle_mirrored_builder("win-trace-rel", True, False, "x64", False, True, views)
angle_mirrored_builder("winuwp-x64-dbg", False, True, "x64", True, False, views)
angle_mirrored_builder("winuwp-x64-rel", False, False, "x64", True, False, views)

# Console views

luci.console_view(
    name = "CI Console View",
    title = "ANGLE CI Builders",
    repo = "https://chromium.googlesource.com/angle/angle",
    refs = ["refs/heads/master"],
    entries = views,
)

# CQ

luci.cq(
    status_host = 'chromium-cq-status.appspot.com',
    submit_max_burst = 4,
    submit_burst_delay = 10 * time.second,
)

luci.cq_group(
    name = 'ANGLE-CQ',
    watch = cq.refset('https://chromium.googlesource.com/angle/angle'),
    acls = [
        acl.entry(
                acl.CQ_COMMITTER,
            groups = 'project-angle-committers',
        ),
        acl.entry(
            acl.CQ_DRY_RUNNER,
            groups = 'project-angle-tryjob-access',
        ),
    ],
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = 'angle:try/presubmit',
            disable_reuse = True,
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/linux-clang-dbg',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/linux-clang-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/mac-dbg',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/mac-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/win-clang-x64-dbg',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/win-clang-x64-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/win-clang-x86-dbg',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/win-clang-x86-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/win-mvcd-x64-dbg',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/win-msvc-x64-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/win-msvc-x86-dbg',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/win-msvc-x86-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/winuwp-x64-dbg',
        ),
        luci.cq_tryjob_verifier(
            builder = 'angle:try/winuwp-x64-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/android_angle_deqp_rel_ng',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/android_angle_rel_ng',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/android_angle_vk32_deqp_rel_ng',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/android_angle_vk64_deqp_rel_ng',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/android_angle_vk32_rel_ng',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/android_angle_vk64_rel_ng',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/fuchsia-angle-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/linux_angle_deqp_rel_ng',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/linux_angle_ozone_rel_ng',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/linux-angle-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/linux-swangle-try-tot-angle-x64',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/mac-angle-rel',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/win-angle-deqp-rel-32',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/win-angle-deqp-rel-64',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/win-angle-rel-32',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/win-angle-rel-64',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/win-swangle-try-tot-angle-x86',
        ),
    ],
)

