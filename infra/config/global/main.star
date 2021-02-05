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

# Recipes

_RECIPE_NAME_PREFIX = "recipe:"

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

# CI bucket

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

luci.gitiles_poller(
    name = "master-poller",
    bucket = "ci",
    repo = "https://chromium.googlesource.com/angle/angle",
    refs = [
        "refs/heads/master",
    ],
    schedule = "with 10s interval",
)

luci.builder(
    name = "linux-clang-dbg",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "linux-clang-rel",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "linux-gcc-dbg",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "linux-gcc-rel",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "linux-trace-rel",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "mac-dbg",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "mac-rel",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "win-clang-x86-dbg",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "win-clang-x86-rel",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "win-clang-x64-dbg",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "win-clang-x64-rel",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "win-msvc-x86-dbg",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "win-msvc-x86-rel",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "win-trace-rel",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "winuwp-x64-dbg",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

luci.builder(
    name = "winuwp-x64-rel",
    bucket = "ci",
    triggered_by = ["master-poller"],
    executable = "recipe:angle",
)

# Try bucket

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
    executable = "recipe:angle",
)

luci.builder(
    name = "linux-clang-dbg",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "linux-clang-rel",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "linux-gcc-dbg",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "linux-gcc-rel",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "linux-trace-rel",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "mac-dbg",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "mac-rel",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "win-clang-x86-dbg",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "win-clang-x86-rel",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "win-clang-x64-dbg",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "win-clang-x64-rel",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "win-msvc-x86-dbg",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "win-msvc-x86-rel",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "win-trace-rel",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "winuwp-x64-dbg",
    bucket = "try",
    executable = "recipe:angle",
)

luci.builder(
    name = "winuwp-x64-rel",
    bucket = "try",
    executable = "recipe:angle",
)

# Console views

luci.console_view(
    name = "CI Console View",
    title = "ANGLE CI Builders",
    repo = "https://chromium.googlesource.com/angle/angle",
    refs = ["refs/heads/master"],
    entries = [
        luci.console_view_entry(
            builder = "ci/linux-clang-dbg",
            category = "linux|clang",
            short_name = "dbg",
        )
    ]
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

