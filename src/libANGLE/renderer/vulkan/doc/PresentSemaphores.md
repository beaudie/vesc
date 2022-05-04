# Queue Present Wait Semaphore Management

The following shorthand notations are used throughout this document:

- PE: Presentation Engine
- ANI: vkAcquireNextImageKHR
- QS: vkQueueSubmit
- QP: vkQueuePresentKHR
- W: Wait
- S: Signal
- R: Render
- P: Present
- SN: Semaphore N
- IN: Swapchain image N
- FN: Fence N

---

## Introduction

Please review required background in [the frame pacing documentation][FramePacing].

As mentioned in the above documentation, ANGLE's frame pacing leads to the following behavior:

```
CPU: ANI QS ANI QS       W:F1 ANI QS     W:F2 ANI QS
     I1  I1 I2  I2            I3  I3          I1  I1
         S:F1   S:F2              S:F3            S:F4
GPU:     <---- R I1 ----><---- R I2 ----><---- R I3 ----><---- R I1 ---->
```

Associated with each QP operation is a semaphore signaled by the preceding QS and waited on by the
PE before the image can be presented.  Currently, there's no feedback from Vulkan (See [internal
Khronos issue][VulkanIssue1060]) regarding _when_ the PE has actually finished waiting on the
semaphore!  This means that the application cannot generally know when to destroy the corresponding
semaphore.  However, taking ANGLE's CPU pacing into account, we are able to destroy (or rather
reuse) semaphores when they are provably unused.

This document describes an approach for destroying semaphores that should work with all valid PE
architectures, but will be described in terms of more common PE architectures (e.g. where the PE
only backs each `VkImage` and `VkSemaphore` handle with one actual memory object, and where the PE
cycles between the swapchain images in a straight-forward manner).

The interested reader may follow the discussion in this abandoned [gerrit CL][CL1757018] for more
background and ideas.

[FramePacing]: FramePacing.md
[VulkanIssue1060]: https://gitlab.khronos.org/vulkan/vulkan/issues/1060
[CL1757018]: https://chromium-review.googlesource.com/c/angle/angle/+/1757018

## Determining When a QP Semaphore is Waited On

Let's combine the diagrams in [the frame pacing documentation][FramePacing] with all the details:

    CPU: ANI   | QS    | QP    | ANI   | QS    | QP    | W:F1 | ANI   | QS    | QP    | W:F2 | ANI   | QS    | QP    | W:F3
         I1    | I1    | I1    | I2    | I2    | I2    |      | I3    | I3    | I3    |      | I1    | I1    | I1    |
         S:SA1 | W:SA1 |       | S:SA2 | W:SA2 |       |      | S:SA3 | W:SA3 |       |      | S:SA4 | W:SA4 |       |
               | S:SP1 | W:SP1 |       | S:SP2 | W:SP2 |      |       | S:SP3 | W:SP3 |      |       | S:SP4 | W:SP4 |
               | S:F1  |       |       | S:F2  |       |      |       | S:F3  |       |      |       | S:F4  |       |
         \_________ ___________/\______________ ______________/\______________ ______________/\______________ ____________/
                   V                           V                              V                              V
                Frame 1                     Frame 2                        Frame 3                        Frame 4

Let's focus only on sequences that return the same image:

    CPU: ANI   | QS    | QP    | W:F(X-1) | ... | ANI   | QS    | QP    | W:F(Y-1) | ANI | QS | QP | W:FY
         I1    | I1    | I1    |          |     | I1    | I1    | I1    |          | I?  | I? | I? |
         S:SAX | W:SAX |       |          |     | S:SAY | W:SAY |       |          |     |    |    |
               | S:SPX | W:SPX |          |     |       | S:SPY | W:SPY |          |     |    |    |
               | S:FX  |       |          |     |       | S:FY  |       |          |     |    |    |
         \_______________ ________________/     \__________________ _______________/\_________ _________/
                         V                                         V                          V
                      Frame X                                   Frame Y                   Frame Y+1

Note that X and Y are arbitrarily distanced (including possibly being sequential).

Say we are at frame Y+1.  There's therefore a wait on FY.  The following holds:

    FY is signaled
    => SAY is signaled
    => The PE has handed I1 back to the application
    => The PE has already processed the *previous* QP of I1
    => SPX is waited on

At this point, we can destroy SPX.  In other words, in frame Y+1, we can destroy SPX (note that 1 is
the number of frames the CPU pacing code uses).  If frame Y+1 is not using I1, this means the
history of present semaphores for I1 would be `{SPX, SPY}` and we can destroy the oldest
semaphore in this list.  If frame Y+1 is also using I1, we should still destroy SPX, but the history
of the present semaphores for I1 would be `{SPX, SPY, SP(Y+1)}`.

We simplify destruction of semaphores by always keeping a history of 3 present semaphores for each
image (3 is H+2 where H is the swap history size used in CPU pacing) and always reuse (instead of
destroy) the oldest semaphore of the image that is about to be presented.

To summarize, we use the completion of a submission using an image to prove when the semaphore used
for the *previous* presentation of that image is no longer in use (and can be safely destroyed or
reused).

## Swapchain recreation

When recreating the swapchain, all images are eventually freed and new ones are created, possibly
with a different count and present mode.  For the old swapchain, we can no longer rely on the
completion of a future submission to know when a previous presentation's semaphore can be destroyed,
as there won't be any more submissions using images from the old swapchain.

> For example, imagine the old swapchain was created in FIFO mode, and one image is being presented
> until the next V-Sync.  Furthermore, imagine the new swapchain is created in MAILBOX mode.  Since
> the old swapchain's image will remain presented until V-Sync, the new MAILBOX swapchain can
> perform an arbitrarily large number of (throw-away) presentations.  The old swapchain (and its
> associated present semaphores) cannot be destroyed until V-Sync; a signal that's not captured by
> Vulkan.

ANGLE resolves this issue by deferring the destruction of the old swapchain and its remaining
present semaphores to the time when the semaphore corresponding to the first present of the new
swapchain can be destroyed.  In the example in the previous section, if SPX is the present semaphore
of the first QP performed on the new swapchain, at frame Y+1, when we know SPX can be destroyed, we
know that the first image of the new swapchain has already been presented.  This proves that all
previous QPs of the old swapchain have been processed.

> Note: the swapchain can potentially be destroyed much earlier, but with no feedback from the
> presentation engine, we cannot know that.  This delay means that the swapchain could be recreated
> while there are pending old swapchains to be destroyed.  The destruction of both old swapchains
> must now be deferred to when the first QP of the new swapchain has been processed.  If an
> application resizes the window constantly and at a high rate, ANGLE would keep accumulating old
> swapchains and not free them until it stops.  While a user will likely not be able to do this (as
> the rate of window system events is lower than the framerate), this can be programmatically done
> (as indeed done in EGL dEQP tests).  Nvidia for example fails creation of a new swapchain if there
> are already 20 allocated (on desktop, or less than ten on Quadro).  If the backlog of old
> swapchains get larger than a threshold, ANGLE calls `vkQueueWaitIdle()` and destroys the
> swapchains.
