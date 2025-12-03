# Troubleshooting OpenCL on RK3588

The error `Failed creating base context during opening of kernel driver` indicates that the Mali GPU kernel driver is not loaded or accessible.

## Diagnosis
The error log shows:
```
arm_release_ver of this libmali is 'g6p0-01eac0', rk_so_ver is '7'.
Failed creating base context during opening of kernel driver.
Kernel module may not have been loaded
OpenCL Error: Failed to create OpenCL context (Error code: -6)
```

This means the user-space library (`libmali.so`) cannot communicate with the kernel-space driver (`mali_kbase`).

## Potential Solutions

1. **Check Kernel Module**:
   Run `lsmod | grep mali` to see if the module is loaded. If not, try to load it:
   ```bash
   sudo modprobe mali_kbase
   ```
   Or search for it:
   ```bash
   find /lib/modules/$(uname -r) -name "*mali*"
   ```

2. **Permissions**:
   Ensure the current user has access to `/dev/mali0` (or similar device file).
   ```bash
   ls -l /dev/mali*
   ```
   You may need to add your user to the `video` or `render` group.

3. **Kernel Version Mismatch**:
   If you updated the kernel recently, the Mali driver might need to be recompiled or reinstalled.

## CPU Benchmark Fallback

I have added a CPU-based OpenMP implementation to the benchmark. You can run it to verify the correctness of the algorithm and get a baseline performance metric.

To build and run with CPU support:
```bash
./build.sh
sudo ./build/benchmark 1920 1080 640 480 100
```
The CPU benchmark will run first, followed by OpenCL (which may fail) and SYCL (if compiled).
