"""
FreeARDU Bare-Metal Build Configuration
Removes PlatformIO's default C runtime that conflicts with custom startup
"""

Import("env", "projenv")

print("[FreeARDU] Configuring bare-metal build...")

# Remove ALL specs that force default startup
if "--specs=nano.specs" in env.get("LINKFLAGS", []):
    env["LINKFLAGS"].remove("--specs=nano.specs")
    print("[FreeARDU] Removed --specs=nano.specs")

if "--specs=nosys.specs" in env.get("LINKFLAGS", []):
    env["LINKFLAGS"].remove("--specs=nosys.specs")
    print("[FreeARDU] Removed --specs=nosys.specs")

# Remove standard libraries that bring in crt0.o
libs_to_remove = ["c", "nosys", "lm", "stdc++"]
new_libs = []
for lib in env.get("LIBS", []):
    if lib not in libs_to_remove:
        new_libs.append(lib)
    else:
        print(f"[FreeARDU] Removed library: {lib}")

env["LIBS"] = new_libs

print("[FreeARDU] Bare-metal linker configured successfully")

print("[FreeARDU] Entry point: reset_handler")


