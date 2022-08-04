# How to Branch and Roll Chromium's ANGLE Dependency

ANGLE provides an implementation of OpenGL ES on Windows, which Chromium relies
upon for hardware accelerated rendering and WebGL support. Chromium specifies
its dependency on a specific version of ANGLE in the repository; this document
describes how to update that dependency, and, if necessary, create an ANGLE
branch to correspond to a branched release of Chrome.

ANGLE's commit queue also runs browser-level tests which are hosted in
the Chromium repository. To reduce the chance of a Chromium-side
change breaking ANGLE's CQ, the version of Chromium against which
ANGLE changes is also snapshotted, and rolled forward into ANGLE with
appropriate testing.

## Autorollers

At present, autorollers manage both the ANGLE roll into Chromium, and
the Chromium roll into ANGLE. All of the ANGLE-related autorollers are
documented in the [ANGLE Wrangling
documentation](../infra/ANGLEWrangling.md#the-auto-rollers).

## Manually rolling Chromium's DEPS on ANGLE

Chromium's dependency on third-party projects is tracked in [the Chromium
repository's src/DEPS file](http://src.chromium.org/viewvc/chrome/trunk/src/DEPS). To update the ANGLE
dependency:

 * Find the line in this file that defines "src/third\_party/angle"
for deps (**not** deps\_os)
 * Change the [git SHA-1 revision
number](http://git-scm.com/book/ch6-1.html) to be that of the commit
on which Chromium should depend. Please use the full SHA-1, not a
shortened version.
 * You can find the SHA-1 for a particular commit with `git log` on the
appropriate branch of the repository, or via [the public repository
viewer](https://chromium.googlesource.com/angle/angle).
 * If using the public repository viewer, you will need to select the
branch whose log you wish to view from the list on the left-hand side,
and then click on the "tree" link at the top of the resulting
page. Alternatively, you can navigate to
`https://chromium.googlesource.com/angle/angle/+/<branch name>/` --
including the terminating forward slash. (e.g.
`https://chromium.googlesource.com/angle/angle/+/main/`)

## Manually rolling ANGLE's DEPS on Vulkan Memory Allocator (VMA)

ANGLE and other Google projects (e.g. Skia, Chrome) use the open-source [Vulkan Memory Allocator][vma-upstream] (VMA)
library.  These projects do not directly use the [upstream Vulkan Memory Allocator][vma-upstream] repository.
Instead, a [Chromium-local repository][vma-chrome] is used, which contains Google-local changes and fixes (e.g. changes
to `BUILD.gn`).  ANGLE's `DEPS` file points to a git SHA-1 revision within the Chromium-local repository.

The [Chromium-local repository][vma-chrome] repository contains the following key branches:

- `main` is manually curated by Google, with a combination of upstream and Google-local changes
- `upstream/master` is automatically mirrored with the contents of the [upstream VMA][vma-upstream] repository

Manual rolls of the `main` branch may require coordination with other Google projects.

Manual rolls of which SHA-1 revision ANGLE's `DEPS` file points to is done by:

- Navigate to the `external/angle` directory
- Check out the `main` branch
- Use `git log` to select the desired Git revision
- Edit the ANGLE `DEPS` file to have the desired revision
- create, upload, and land a CL with this change

[vma-upstream]: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
[vma-chrome]: https://chromium.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator

Note: When ANGLE is AutoRolled to the Android AOSP source tree, Google-local
changes to the VMA `BUILD.gn` file will be converted to the ANGLE `Android.bp` file.

## Branching ANGLE

Sometimes, individual changes to ANGLE are needed for a release of Chrome which
has already been branched. If this is the case, a branch of ANGLE should be
created to correspond to the Chrome release version, so that Chrome may
incorporate only these changes, and not everything that has been committed since
the version on which Chrome depended at branch time. **Please note: Only ANGLE
admins can create a new branch.** To create a branch of ANGLE for a branched
Chrome release:

 * Determine what the ANGLE dependency is for the Chrome release
by checking the DEPS file for that branch.
 * Check out this commit as a new branch in your local repository.
   * e.g., for [the Chrome 34 release at
chrome/branches/1847](http://src.chromium.org/viewvc/chrome/branches/1847/src/DEPS),
the ANGLE version is 4df02c1ed5e97dd54576b06964b1da67ea30238e. To
check this commit out locally and create a new branch named 'mybranch'
from this commit, use: ```git checkout -b mybranch
4df02c1ed5e97dd54576b06964b1da67ea30238e```
 * To create this new branch in the public repository, you'll need to push the
branch to the special Gerrit reference location, 'refs/heads/<branch name>'. You
must be an ANGLE administrator to be able to push this new branch.
    * e.g., to use your local 'mybranch' to create a branch in the public repository called
'chrome\_m34', use: ```git push origin mybranch:refs/heads/chrome_m34```
    * The naming convention that ANGLE uses for its release-dedicated branches is 'chrome\_m##'.
