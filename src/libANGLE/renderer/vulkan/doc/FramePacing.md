# Frame Pacing (a.k.a Throttling)

The following shorthand notations are used throughout this document:

- PE: Presentation Engine
- ANI: vkAcquireNextImageKHR
- QS: vkQueueSubmit
- QP: vkQueuePresentKHR
- W: Wait
- S: Signal
- CB: Record command buffers
- R: Render
- P: Present
- SN: Semaphore N
- IN: Swapchain image N
- FN: Fence N

---

Vulkan requires the application (ANGLE in this case) to acquire swapchain images and queue them for
presentation, synchronizing GPU submissions with semaphores.  A single frame looks like the
following (Note: `S:S1` signifies signaling `S1`, and `W:S1` signifies waiting on it):

```
CPU: <---- CB ----->
           ANI  ... QS   ... QP
           S:S1     W:S1     W:S2
                    S:S2
GPU:                     <------------ R ----------->
                         ^
                     S1 signaled
 PE:                                                      <-------- P ------>
                                                          ^
                                                   S2 signaled and
                                            PE ready (for example on v-sync)
```

That is:

- The CPU records command buffers, at which time input is taken into account, including animation
  time,
- The GPU starts rendering after submission, and
- The presentation is started after rendering is finished.

Note that Vulkan tries to abstract a large variety of PE architectures, some of which do not behave
in a straight-forward manner.  As such, ANGLE cannot know what the PE is exactly doing with the
images or when the images are visible on the screen.  The only signal out of the PE is received
through the semaphore that's used in ANI.

With multiple frames, the pipeline looks different based on present mode.  Let's focus on
FIFO (the arguments in this document translate to all modes) with 3 images:

```
CPU: CB QS QP CB QS QP CB QS QP CB QS QP
        I1 I1    I2 I2    I3 I3    I1 I1
GPU: <---- R I1 ----><---- R I2 ----><---- R I3 ----><---- R I1 ---->
 PE:                 <----- P I1 -----><----- P I2 -----><----- P I3 -----><----- P I1 ----->
```

An issue is evident here.  The CPU is submitting jobs and queuing images for presentation faster
than the GPU can render them or the PE can view them.  On some systems, this can cause the length of
the submit queue to grow indefinitely, resulting in larger and larger input lag.  In FIFO mode, the
PE present queue may also grow indefinitely, or at least grow long enough for input lag to become
perceptible.

To address this issue, ANGLE paces the CPU such that the recording of frame `N+2` is done while
frame `N+1` is being rendered and frame `N` is being presented.  This minimizes the input lag.

```
CPU: CB1 QS CB2 QS       W:F1 CB3 QS     W:F2 CB4 QS     W:F3
         I1     I2                I3              I1
         S:F1   S:F2              S:F3            S:F4
GPU:     <---- R I1 ----><---- R I2 ----><---- R I3 ----><---- R I1 ---->
```

> Note: Ideally, the length of the PE present queue should also be kept at a maximum of 1 (i.e. one
> image being presented, and another in queue).  However, the Vulkan WSI extension doesn't provide
> enough control to achieve this.
